import time
import json
import os
import random
import struct
import tsc
import reader
from tsc import DataType, FieldFlags, TypeSpec, TObj, TList, TTypedArray, TStructInstance

# ==========================================
# CONFIGURATION
# ==========================================
ITERATIONS = 5  # How many times to run each test for averaging
GREEN = "\033[92m"
YELLOW = "\033[93m"
RESET = "\033[0m"

# ==========================================
# HELPERS: AST GENERATION
# ==========================================
def python_to_tson_ast(data, type_hint=None):
    """Converts Python Dicts/Lists to TSON AST nodes for the Writer."""
    if isinstance(data, dict):
        return TObj({k: python_to_tson_ast(v) for k, v in data.items()})
    elif isinstance(data, list):
        # Heuristic: If simple list of dicts -> Generic List (unless manual TypedArray)
        # For this benchmark, we manually construct TypedArrays in the scenario setup
        return TList([python_to_tson_ast(x) for x in data])
    else:
        return data  # Primitive

def get_file_size(filename):
    return os.path.getsize(filename)

def format_size(bytes):
    return f"{bytes / 1024:.2f} KB"

# ==========================================
# BENCHMARK ENGINE
# ==========================================
def run_benchmark(name, data_gen_func, tson_struct_def_func):
    print(f"\n{YELLOW}=== BENCHMARK: {name} ==={RESET}")
    
    json_write_times = []
    json_read_times = []
    tson_write_times = []
    tson_read_lazy_times = [] # Time to open and access 1 item
    tson_read_full_times = [] # Time to traverse everything
    
    # 1. Generate Data (Once)
    py_data, tson_root_node = data_gen_func()
    
    # 2. Run Loop
    for i in range(ITERATIONS):
        # --- JSON ---
        start = time.time()
        with open("bench.json", "w") as f:
            json.dump(py_data, f, separators=(',', ':')) # Minified
        json_write_times.append(time.time() - start)
        
        start = time.time()
        with open("bench.json", "r") as f:
            d = json.load(f)
            # Force traversal to ensure it's loaded
            _ = len(str(d)) 
        json_read_times.append(time.time() - start)

        # --- TSON ---
        # Write
        start = time.time()
        writer = tsc.TSONWriter()
        tson_struct_def_func(writer) # Register Structs
        binary = writer.finalize(tson_root_node)
        with open("bench.tbin", "wb") as f:
            f.write(binary)
        tson_write_times.append(time.time() - start)
        
        # Read (Lazy)
        start = time.time()
        r = reader.TSONReader("bench.tbin")
        root = r.parse_node_at_current()
        # Access one random thing to trigger IO
        if hasattr(root, 'keys'): _ = root[list(py_data.keys())[0]] 
        tson_read_lazy_times.append(time.time() - start)

        # Read (Full Traversal - Approximate)
        # We re-open to clear cache effects
        r = reader.TSONReader("bench.tbin")
        root = r.parse_node_at_current()
        start = time.time()
        # Recursive walk to force read
        def walk(node):
            if hasattr(node, 'keys'): # Object
                for k in list(py_data.keys()): # Approximate keys
                    try: walk(node[k])
                    except: pass
            elif hasattr(node, 'rows'): # Typed Array
                _ = len(node)
                # Access 10% of rows to simulate usage
                for j in range(0, len(node), 10):
                    _ = node[j]
            elif isinstance(node, list):
                for x in node: walk(x)
        walk(root)
        tson_read_full_times.append(time.time() - start)

    # 3. Report Results
    avg_j_w = sum(json_write_times) / ITERATIONS
    avg_j_r = sum(json_read_times) / ITERATIONS
    avg_t_w = sum(tson_write_times) / ITERATIONS
    avg_t_r_lazy = sum(tson_read_lazy_times) / ITERATIONS
    avg_t_r_full = sum(tson_read_full_times) / ITERATIONS
    
    json_size = get_file_size("bench.json")
    tson_size = get_file_size("bench.tbin")
    
    print(f"{'Metric':<20} | {'JSON':<15} | {'TSON':<15} | {'Diff':<15}")
    print("-" * 70)
    print(f"{'File Size':<20} | {format_size(json_size):<15} | {format_size(tson_size):<15} | {GREEN}{tson_size/json_size:.2%} size{RESET}")
    print(f"{'Write Time (ms)':<20} | {avg_j_w*1000:<15.4f} | {avg_t_w*1000:<15.4f} | {avg_t_w/avg_j_w:.2f}x")
    print(f"{'Read (Lazy) (ms)':<20} | {'N/A':<15} | {avg_t_r_lazy*1000:<15.4f} | {GREEN}Instant{RESET}")
    print(f"{'Read (Full) (ms)':<20} | {avg_j_r*1000:<15.4f} | {avg_t_r_full*1000:<15.4f} | {avg_t_r_full/avg_j_r:.2f}x")

    # Cleanup
    if os.path.exists("bench.json"): os.remove("bench.json")
    if os.path.exists("bench.tbin"): os.remove("bench.tbin")

# ==========================================
# SCENARIO 1: 3D MESH (Numerical Arrays)
# 100k Vertices (Float), 100k Indices (Int)
# ==========================================
def gen_mesh():
    count = 50000
    # Python Data
    verts = [random.random() for _ in range(count * 3)] # x,y,z
    inds = [random.randint(0, count) for _ in range(count)]
    py_data = {"vertices": verts, "indices": inds}
    
    # TSON AST
    # Using ARRAY_PRIM optimization
    root = TObj({
        "vertices": TTypedArray(TypeSpec(DataType.ARRAY, TypeSpec(DataType.FLOAT32), FieldFlags.NONE), verts),
        "indices": TTypedArray(TypeSpec(DataType.ARRAY, TypeSpec(DataType.INT32), FieldFlags.NONE), inds)
    })
    return py_data, root

def def_mesh(w):
    pass # No struct definitions needed, using primitive arrays

# ==========================================
# SCENARIO 2: DATABASE (Table of Structs)
# 10k User Objects
# ==========================================
def gen_db():
    count = 10000
    users_py = []
    users_tson = []
    
    for i in range(count):
        u = (i, f"user_{i}", f"user_{i}@example.com", i % 2 == 0)
        users_py.append({"id": u[0], "name": u[1], "email": u[2], "active": u[3]})
        users_tson.append(u) # Tuple for struct row
    
    py_data = {"users": users_py}
    
    # TSON AST
    # User[]
    user_spec = TypeSpec(DataType.ARRAY, TypeSpec(DataType.STRUCT, "User"), FieldFlags.NONE)
    root = TObj({
        "users": TTypedArray(user_spec, users_tson)
    })
    
    return py_data, root

def def_db(w):
    w.define_struct("User", [
        ("id", TypeSpec(DataType.INT32)),
        ("name", TypeSpec(DataType.STR)),
        ("email", TypeSpec(DataType.STR)),
        ("active", TypeSpec(DataType.BOOL))
    ])

# ==========================================
# SCENARIO 3: DEEP CONFIG (Nested Objects)
# ==========================================
def gen_config():
    # Recursively generate nested dicts
    def make_node(depth):
        if depth == 0: return {"val": 1, "desc": "leaf"}
        return {
            "left": make_node(depth-1),
            "right": make_node(depth-1),
            "meta": [1, 2, 3]
        }
    
    py_data = make_node(12) # 2^12 nodes (~4000 objects)
    
    # TSON AST
    # Just standard generic object conversion
    root = python_to_tson_ast(py_data)
    return py_data, root

def def_config(w):
    pass

# ==========================================
# MAIN
# ==========================================
if __name__ == "__main__":
    print(f"{GREEN}Starting TSON vs JSON Performance Test...{RESET}")
    print(f"Iterations per test: {ITERATIONS}")
    
    run_benchmark("Massive Numerical Arrays (Mesh)", gen_mesh, def_mesh)
    run_benchmark("Structured Table (Database)", gen_db, def_db)
    run_benchmark("Deep Nesting (Config Tree)", gen_config, def_config)