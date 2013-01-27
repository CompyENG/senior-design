# RGB lookup table generator
import numpy
# U, V are signed -- Y is unsigned

def clip(val):
    if val < 0:
        return 0
    elif val > 255:
        return 255
    else:
        return val


def yuv_to_r(y, u, v):
    return clip( ((y << 12)            + 5742 * v + 2048) >> 12 )
    
def yuv_to_g(Y, U, V):
    return clip( ((Y << 12) - 1411 * U - 2925 * V + 2048) >> 12 )
    
def yuv_to_b(Y, U, V):
    return clip( ((Y << 12) + 7258 * U            + 2048) >> 12 )
    
print "Generating red table..."
# Red table
#  r[y,v] in python, r[y][v] in C
r = numpy.zeros(shape=(256, 256), dtype=numpy.uint8)
for y in range(0, 256):
    for v in range(-128, 128):
        r[y,v+128] = yuv_to_r(y, 0, v) # Passing 0 because u doesn't matter
        
print "Generating green table..."
# Green table
#  g[y,u,v] in python, g[y][u][v] in C
g = numpy.zeros(shape=(256,256,256), dtype=numpy.uint8)
for y in range(0, 256):
    for u in range(-128, 128):
        for v in range(-128, 128):
            g[y,u+128,v+128] = yuv_to_g(y,u,v)
            
print "Generating blue table..."
# Blue table
#  b[y,u] in python, b[y][u] in C
b = numpy.zeros(shape=(256,256), dtype=numpy.uint8)
for y in range(0, 256):
    for u in range(-128, 128):
        b[y,u+128] = yuv_to_b(y,u,0) # Passing 0 because v doesn't matter


f = open('lookup.h', 'w')
print "Writing red table..."
# Small note: I would like static const, but can't have that with extern
# Write red table
f.write('// Red lookup table -- r[y][v]\n')
f.write('static const uint8_t r[256][256] = { \n')
for i in range(0, r.shape[0]):
    f.write('  { ')
    k = 0
    for j in range(0, r.shape[1]):
        f.write('%3d, ' % (r[i,j],))
        k += 1
        if k == 16:
            f.write('\n    ')
            k = 0
    f.write('},\n')
f.write('  };\n\n')

# Write green table
print "Writing green table..."
f.write('// Green lookup table -- g[y][u][v]\n')
f.write('static const uint8_t g[256][256][256] = { \n')
for i in range(0, g.shape[0]):
    f.write('  { ')
    for j in range(0, g.shape[1]):
        f.write('  { ')
        h = 0
        for k in range(0, g.shape[2]):
            f.write('%3d, ' % (g[i,j,k],))
            h += 1
            if h == 16:
                f.write('\n    ')
                h = 0
        f.write('},\n')
    f.write('},\n')
f.write('  };\n\n')

# Write blue table
print "Writing blue table..."
f.write('// Blue lookup table -- b[y][u]\n')
f.write('static const uint8_t b[256][256] = { \n')
for i in range(0, b.shape[0]):
    f.write('  { ')
    k = 0
    for j in range(0, b.shape[1]):
        f.write('%3d, ' % (b[i,j],))
        k += 1
        if k == 16:
            f.write('\n    ')
            k = 0
    f.write('},\n')
f.write('  };\n\n')

f.close()