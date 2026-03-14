import sys

mtxfile = sys.argv[1]
graph_name = mtxfile.replace(".mtx", "")

src_to_dst = {}
dst_to_src = {}

# mtx attributes;
symmetric = False

V = None
E = None

# Parse mtx file
with open(mtxfile, "r") as file0:

  # Parse the first line;
  first_line = file0.readline().strip()
  if first_line.startswith("%%"):
    words = first_line.split()
  else:
    print("Error: the first line does not start with %%.")
    sys.exit(1)

  VE_found = False

  for line in file0:
    stripped = line.strip()
    if stripped.startswith("%"):
      continue
    else:
      if VE_found: # every line after the first (after header) should be an edge;
        words = stripped.split()
        src = int(words[0])
        dst = int(words[1])
        src -= 1
        dst -= 1
        if src == dst:
          continue
        src_to_dst[src].append(dst)
        dst_to_src[dst].append(src)
      else: # Only the first line after the header should have V, E;
        VE_found = True
        words = stripped.split()
        V0 = int(words[0])
        V1 = int(words[1])
        # E  = int(words[2])
        #if symmetric:
        #  E *= 2
        if V0 != V1:
          print("Error: V0, V1 do not match")
          sys.exit(1)
        else:
          V = V0

        src_to_dst = {v: [] for v in range(V)}
        dst_to_src = {v: [] for v in range(V)}

# count edges
e0 = 0
e1 = 0
for v in range(V):
  e0 += len(src_to_dst[v])
  e1 += len(dst_to_src[v])

if e0 != e1:
  print("Error: e0, e1 do not match.")
  sys.exit()
else:
  E = e0

print("V={}".format(V))
print("E={}".format(E))


# sort;     
for v in range(V):
  src_to_dst[v].sort()
  dst_to_src[v].sort()
  
  

# write fwd offsets / nonzeros;
rev_file = open("{}-rev.mtx".format(graph_name), "w")
rev_file.write("%%MatrixMarket matrix coordinate real general\n")
rev_file.write("%\n")
rev_file.write(f"{V} {V} {E}\n")
for dst in range(V):
  srcs = dst_to_src[dst]
  for src in srcs:
    rev_file.write(f"{dst} {src} 1\n")