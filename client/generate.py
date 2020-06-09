import json
from shutil import copyfile

# data = {}
j = open("test_data.json", "r")
data = json.load(j)

# 1. Parsing data
# a) Figure out the relevant location range
#   - This is our min location to our max location + size allocated at that point
# b) Figure out the relevant time range [DONE]
#   - This is start_ts, up until max ts

# 2. Generate 2-dimensional heatmap of memory allocation over time
# Values are already sorted by timestamp
# a) At each timestamp, generate a new array with the currently allocated memory

start_ts = data['start_ts']
end_ts = max(value['ts'] for value in data['values'])

# https://stackoverflow.com/questions/5914627/prepend-line-to-beginning-of-a-file
def line_prepender(filename, line):
    with open(filename, 'r+') as f:
        content = f.read()
        f.seek(0, 0)
        f.write(line.rstrip('\r\n') + '\n' + content)

def generate_file():
  copyfile('template.html', 'index.html')

  dataString = f'''<script>
    const data = {data}
    </script>'''

  line_prepender('index.html', dataString)

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

