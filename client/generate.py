import json
from shutil import copyfile

# For gaps greater than this number, we render an ellipsis instead of white space.
GAP_THRESHOLD = 100

# data = {}
j = open("test_data.json", "r")
data = json.load(j)

# 1. Parsing data
# a) Figure out the relevant location range [DONE]
#   - [min location, max location + size allocated at that value]
# b) Figure out the relevant time range [DONE]
#   - [start_ts, max ts]

# 2. Generate 2-dimensional heatmap of memory allocation over time
# note: Values are already sorted by timestamp
# a) Sort by location [DONE]
# b) Start building an array of 0s from the beginning, if there's a gap, insert a -1 [DONE]
# c) Build a dict to map location onto array index [DONE]
# d) Walk through values, generating a new array for each timestamp

values = data['values']

start_ts = data['start_ts']
end_ts = max(value['ts'] for value in values)

start_l = min(value['l'] for value in values)
end_l = max(value['l'] + value['s'] for value in values)
 
memory_arr = []
location_to_index = {}
sorted_l = sorted(values, key=lambda value: value['l'])
last_l = sorted_l[0]['l']

for value in sorted_l:
  l = value['l']
  location_to_index[l] = len(memory_arr)
  gap = l - last_l
  memory_arr.extend([0] * value['s'])
  if gap >= GAP_THRESHOLD:
    memory_arr.append(-1)
  else:
    memory_arr.extend([0] * gap)
  last_l = value['l']

heatmap_data = [memory_arr.copy()]
for value in values:
  l = value['l']
  s = value['s']
  new_arr = heatmap_data[-1].copy()
  index = location_to_index[l]
  for i in range(s):
    new_arr[index + i] = 1
  heatmap_data.append(new_arr)

# print(memory_arr)
print(heatmap_data[-1])
print(len(heatmap_data))
print(len(heatmap_data[0]))


# https://stackoverflow.com/questions/5914627/prepend-line-to-beginning-of-a-file
def line_prepender(filename, line):
    with open(filename, 'r+') as f:
        content = f.read()
        f.seek(0, 0)
        f.write(line.rstrip('\r\n') + '\n' + content)

def generate_file():
  copyfile('template.html', 'index.html')

  dataString = f'''<script>
  const data = {heatmap_data}
</script>'''

  line_prepender('index.html', dataString)

generate_file()

# # print(j.read())

# # with open('./data.json', 'r') as file:
# #   json.dump(data, file)


# f = open("index.html", "w")
# f.write("cat dog Now the file has more content 2!")
# f.close()

# Scroll right to go through time in program execution
# Linear mapping start to turn red
# In the middle of the program everything should be red
# Click into the rectangle to see the actual memory layout of that page
# Could have a legend on the right side of the heap visualization to show different colors

# ts is timestamp
# l is location
# sh is scope, don't worry about this
# th types, doesn't matter right now
# s is size, number of bytes

# In the future, we'll know which thread something came from

# Inside of Erata, two subdirectories
# One for allocation, one for deallocation

