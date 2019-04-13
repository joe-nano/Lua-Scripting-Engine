material = LoadMaterial()

x = { dave = "busy", ian = "idle"}

vec1 = CreateVector()
vec2 = CreateVector()

print(vec1.x.." "..vec1.y.."  "..vec1.z)

vec1.x = 5
vec1.y = 2
vec1.z = 1

print(vec1.x.." "..vec1.y.."  "..vec1.z)

vec2.x = 3
vec2.y = 6
vec2.z = 9

vec3 = vec1 + vec2


print("vector3:  "..vec3.x.." "..vec3.y.."  "..vec3.z)