# FlowCV Python Scripting Node

Adds Python scripting functionality to a node.

#### Some things to keep in mind

* This is still very experimental and may be glitchy or unstable (save often).
* I did my best to keep everything thread safe while maximizing the multi-threading capabilities between multiple nodes but due to certain limitations with pybind11 and memory sharing/access along with other reasons, each individual instance of the node will be limited to a single thread.
* While the Python scripting node allows for a lot a flexibility it may adversely affect performance so use the nodes sparingly and keep the processing to a minimum, ideally if you come up with a good python workflow that you use a lot consider turning it into a custom C++ node or break it down to the component parts and create the necessary C++ nodes to recreate the same functionality without the Python overhead. 


---

### Build Instructions

Prerequisites
* Clone [FlowCV](https://github.com/FlowCV-org/FlowCV) repo.
* Requires Python 3.6 or newer (with developer files)
* _[pybind11](https://github.com/pybind/pybind11) and other dependecies will download automatically using CMake Fetch_


Build Steps:
1. Clone this repo.
2. cd to the repo directory
3. Run the following commands:

```shell
mkdir Build
cd Build
cmake .. -DFlowCV_DIR=/path/to/FlowCV/FlowCV_SDK
make
```

---

### Usage

* On Windows you may have to manually set the python interpreter location, the plugin will attempt to find it automatically if you have the PYTHONHOME environment variable set or if it can find a valid python path in the PATH environment variable.
* On Linux and MacOS it should detect python automatically.

The node can be found under the experimental section (as this is still very much experimental and may be glitchy or unstable so save often):

![addNode](Docs/images/add_python_node.jpg)

Node:

![node](Docs/images/python_node.jpg)

The node by default will pass any inputs straight through to the outputs if the script does not do anything to modify them in between.

You have various inputs and outputs that can be accessed through the python module that will be explained below.

Node Options:

![nodeOptions](Docs/images/python_node_options.jpg)

1. Set your python interpreter (on Windows) if not automatically set
2. Select a python script to run

Any errors or other output will go to the console, since it runs for every frame if you have a lot of output or errors you will get a lot of scrolling text very quickly, so it's best to avoid printing anything out within the python script.

if you want to run additional OpenCV processing within the script you will need to have the opencv-python library and numpy installed within your python interpreter environment.

---

### FlowCV Python Module

The python module is named: flowcv
```python
import flowcv
```
To access the data class of the node simply grab a pointer to the class using the local active node variable provided:
```python
data = flowcv.node_data(locals()['active_node'])
```
| Method      | Parameter     | Return Type | Description            |
|:------------|:--------------|:------------|:-----------------------|
| `node_data` | `active_node` | `flowData`    | FlowCV Node Data Class |


Once you have the node data class you can access the various inputs and outputs through the class methods.

| Method       | Parameter          | Return Type | Description                                                                   |
|:-------------|:-------------------|:------------|:------------------------------------------------------------------------------|
| `get_image`  | `int`              | `Image`     | FlowCV cv::Mat Image Wrapper Class, 0 = img1 input, 1 = img2 input            |
| `set_image`  | `numpy.array, int` | `none`      | Set the output image and specify which output to use 0 = img1, 1 = img2       |
| `get_bool`   | `none`             | `bool`      | Get bool input                                                                |
| `set_bool`   | `bool`             | `none`      | Set bool output                                                               |
| `get_int`    | `none`             | `int`       | Get int input                                                                 |
| `set_int`    | `int`              | `none`      | Set int output                                                                |
| `get_float`  | `none`             | `float`     | Get float input                                                               |
| `set_float`  | `float`            | `none`      | Set float output                                                              |
| `get_string` | `none`             | `string`    | Get string input                                                              |
| `set_string` | `string`           | `none`      | Set string output                                                             |
| `get_json`   | `none`             | `dict`      | Get JSON input                                                                |
| `set_json`   | `dict`             | `none`      | Set JSON output                                                               |


Image data will need to be converted to a valid numpy array before using with OpenCV (use copy=False to avoid unnecessary copying of data):
```python
image = numpy.array(data.get_image(0), copy=False)
```

JSON data will be automatically converted between a python dictionary and C++ JSON object.
If there is no JSON input going into the node then the returned object will be a python None Type object, so it's best to check first if the returned object is None and initialize it if necessary:
```python
j = data.get_json()
if j == None:
    j = {}
j['val2'] = 55
data.set_json(j)
```

---

### Example

```python
import numpy as np
import cv2
import flowcv

# Get flowcv data class
data = flowcv.node_data(locals()['active_node'])

# Convert img1 input to OpenCV compatible numpy array
image = np.array(data.get_image(0), copy=False)

# Create a new blank image
blank_image = np.zeros((480, 640, 3), np.uint8)

# Make the left half blue
blank_image[:, 0:640//2] = (255, 0, 0)

# Grab the incoming JSON data if any
j = data.get_json()

# Check if it is None and if so initialize dictionary
if j == None:
    j = {}

# Add some new data to the dictionary
j['val2'] = 55

# Set the JSON output
data.set_json(j)

# Set the bool output
data.set_bool(True)

# Set the string output
data.set_string("Hello World")

# Blur the incoming image
image = cv2.blur(image, (32, 32))

# Set the processed image to img1 output
data.set_image(image, 0)

# Set the new blank image to img2 output
data.set_image(blank_image, 1)
```
