# ESP DL MicroPython Binding

This is a MicroPython binding for ESP-DL (Deep Learning) models that enables face detection, face recognition, human detection, cat detection, and image classification on ESP32 devices.

## Donate

I spent a lot of time and effort to make this. If you find this project useful, please consider donating to support my work.
[![Donate](https://img.shields.io/badge/Donate-PayPal-blue.svg)](https://www.paypal.me/cnadler)

## Available Models

- `FaceDetector`: Detects faces in images and provides bounding boxes and facial features
- `FaceRecognizer`: Recognizes enrolled faces and manages a face database
- `HumanDetector`: Detects people in images and provides bounding boxes
- `CatDetector`: Detects cats in images and provides bounding boxes
- `ImageNet`: Classifies images into predefined categories
- `CocoDetector`: Detects objects in images using COCO dataset categories

## Installation & Building

### Requirements

- ESP-IDF:
  - Version 5.4.2 with MicroPython >=1.26.0
- Make sure you have the complete ESP32 build environment set up

### Precompiled Images

You can find precompiled images in two ways:
1. In the Actions section for passed workflows under artifacts
2. By forking the repo and manually starting the action

### Building from Source

1. Clone the required repositories:
```sh
git clone --recursive https://github.com/cnadler86/mp_esp_dl_models.git
git clone https://github.com/cnadler86/micropython-camera-API.git
git clone https://github.com/cnadler86/mp_jpeg.git
```

2. Build the firmware:
There are two ways to enable the different models:

a) Using mpconfigvariant files (recommended):
The models can be enabled in the board's mpconfigvariant files (e.g., mpconfigvariant_FLASH_16M.cmake). The following flags are available:
- MP_DL_FACE_DETECTOR_ENABLED
- MP_DL_FACE_RECOGNITION_ENABLED
- MP_DL_PEDESTRIAN_DETECTOR_ENABLED
- MP_DL_IMAGENET_CLS_ENABLED
- MP_DL_COCO_DETECTOR_ENABLED
- MP_DL_CAT_DETECTOR_ENABLED

b) Using command line flags:
You can enable models directly through the idf.py command using -D flags:
```sh
idf.py -D MP_DL_FACE_RECOGNITION_ENABLED=1 -D MP_DL_CAT_DETECTOR_ENABLED=1 [other flags...]
```

Basic build command:
```sh
cd mp_esp_dl_models/boards/
idf.py -D MICROPY_DIR=<micropython-dir> -D MICROPY_BOARD=<BOARD_NAME> -D MICROPY_BOARD_VARIANT=<BOARD_VARIANT> -B build-<your-build-name> build
cd build-<your-build-name>
python ~/micropython/ports/esp32/makeimg.py sdkconfig bootloader/bootloader.bin partition_table/partition-table.bin micropython.bin firmware.bin micropython.uf2
```

## Module Usage

### Common Requirements

All models support various input pixel formats including RGB888 (default), RGB565, and others supported by ESP-DL. You can use [mp_jpeg](https://github.com/cnadler86/mp_jpeg/) to decode camera images to the correct format.

The pixel format can be set through the constructor's `pixel_format` parameter. This value matches the ESP-DL image format definitions.

### FaceDetector

The FaceDetector module detects faces in images and can optionally provide facial feature points.

#### Constructor
```python
FaceDetector(width=320, height=240, pixel_format=DL_IMAGE_PIX_TYPE_RGB888, features=True)
```

**Parameters:**
- `width` (int, optional): Input image width. Default: 320
- `height` (int, optional): Input image height. Default: 240
- `pixel_format` (int, optional): Input image pixel format. Default: DL_IMAGE_PIX_TYPE_RGB888
- `features` (bool, optional): Whether to return facial feature points. Default: True

#### Methods

- **run(framebuffer)**
  
  Detects faces in the provided image.

  **Parameters:**
  - `framebuffer`: RGB888 image data (required)

  **Returns:**
  List of dictionaries with detection results, each containing:
  - `score`: Detection confidence (float)
  - `box`: Bounding box coordinates [x1, y1, x2, y2]
  - `features`: Facial feature points [(x,y) coordinates for: left eye, right eye, nose, left mouth, right mouth] if enabled, None otherwise

### FaceRecognizer

The FaceRecognizer module manages a database of faces and can recognize previously enrolled faces.

#### Constructor
```python
FaceRecognizer(width=320, height=240, pixel_format=DL_IMAGE_PIX_TYPE_RGB888, features=True, db_path="face.db", model=None)
```

**Parameters:**
- `width` (int, optional): Input image width. Default: 320
- `height` (int, optional): Input image height. Default: 240
- `pixel_format` (int, optional): Input image pixel format. Default: DL_IMAGE_PIX_TYPE_RGB888
- `features` (bool, optional): Whether to return facial feature points. Default: True
- `db_path` (str, optional): Path to the face database file. Default: "face.db"
- `model` (str, optional): Feature extraction model to use ("MBF" or "MFN"). Default: None (uses default model)

#### Methods

- **run(framebuffer)**
  
  Detects and recognizes faces in the provided image.

  **Parameters:**
  - `framebuffer`: RGB888 image data (required)

  **Returns:**
  List of dictionaries with recognition results, each containing:
  - `score`: Detection confidence
  - `box`: Bounding box coordinates [x1, y1, x2, y2]
  - `features`: Facial feature points (if enabled)
  - `person`: Recognition result containing:
    - `id`: Face ID
    - `similarity`: Match confidence (0-1)
    - `name`: Person name (if provided during enrollment)

- **enroll(framebuffer, validate=False, name=None)**
  
  Enrolls a new face in the database.

  **Parameters:**
  - `framebuffer`: RGB888 image data
  - `validate` (bool, optional): Check if face is already enrolled. Default: False
  - `name` (str, optional): Name to associate with the face. Default: None

  **Returns:**
  - ID of the enrolled face

- **delete_face(id)**
  
  Deletes a face from the database.

  **Parameters:**
  - `id` (int): ID of the face to delete

- **print_database()**
  
  Prints the contents of the face database.

### HumanDetector and Cat Detector

The HumanDetector module detects people in images. The CatDetector does it for cats. Both modules provide bounding boxes for detected objects.

#### Constructor
```python
HumanDetector(width=320, height=240, pixel_format=DL_IMAGE_PIX_TYPE_RGB888) #For cats use CatDetector
```

**Parameters:**
- `width` (int, optional): Input image width. Default: 320
- `height` (int, optional): Input image height. Default: 240
- `pixel_format` (int, optional): Input image pixel format. Default: DL_IMAGE_PIX_TYPE_RGB888

#### Methods

- **run(framebuffer)**
  
  Detects people in the provided image.

  **Parameters:**
  - `framebuffer`: RGB888 image data

  **Returns:**
  List of dictionaries with detection results, each containing:
  - `score`: Detection confidence
  - `box`: Bounding box coordinates [x1, y1, x2, y2]

### ImageNet

The ImageNet module classifies images into predefined categories.

#### Constructor
```python
ImageNet(width=320, height=240, pixel_format=DL_IMAGE_PIX_TYPE_RGB888)
```

**Parameters:**
- `width` (int, optional): Input image width. Default: 320
- `height` (int, optional): Input image height. Default: 240
- `pixel_format` (int, optional): Input image pixel format. Default: DL_IMAGE_PIX_TYPE_RGB888

#### Methods

- **run(framebuffer)**
  
  Classifies the provided image.

  **Parameters:**
  - `framebuffer`: RGB888 image data

  **Returns:**
  List alternating between class names and confidence scores:
  `[class1, score1, class2, score2, ...]`

### COCO detect

The COCO detect module detects objects in images using the COCO dataset.

#### Constructor
```python
COCODetector(width=320, height=240, pixel_format=DL_IMAGE_PIX_TYPE_RGB888, model=CONFIG_DEFAULT_COCO_DETECT_MODEL)
```

**Parameters:**
- `width` (int, optional): Input image width. Default: 320
- `height` (int, optional): Input image height. Default: 240
- `pixel_format` (int, optional): Input image pixel format. Default: DL_IMAGE_PIX_TYPE_RGB888
- `model` (int, optional): COCO detection model to use. Default: CONFIG_DEFAULT_COCO_DETECT_MODEL

#### Methods
- **run(framebuffer)**
  
  Detects objects in the provided image.

  **Parameters:**
  - `framebuffer`: RGB888 image data

  **Returns:**
  List of dictionaries with detection results, each containing:
  - `score`: Detection confidence
  - `box`: Bounding box coordinates [x1, y1, x2, y2]
  - `category`: Detected object class id

## Usage Examples

### Face Detection Example
```python
from espdl import FaceDetector
import camera
from jpeg import Decoder

# Initialize components
cam = camera.Camera()
decoder = Decoder()
face_detector = FaceDetector()

# Capture and process image
img = cam.capture()
framebuffer = decoder.decode(img)  # Convert to RGB888
results = face_detector.run(framebuffer)

if results:
    for face in results:
        print(f"Face detected with confidence: {face['score']}")
        print(f"Bounding box: {face['box']}")
        if face['features']:
            print(f"Facial features: {face['features']}")
```

### Face Recognition Example
```python
from espdl import FaceRecognizer
import camera
from jpeg import Decoder

# Initialize components
cam = camera.Camera()
decoder = Decoder()
recognizer = FaceRecognizer(db_path="/faces.db")

# Enroll a face
img = cam.capture()
framebuffer = decoder.decode(img)
face_id = recognizer.enroll(framebuffer, name="John")
print(f"Enrolled face with ID: {face_id}")

# Later, recognize faces
img = cam.capture()
framebuffer = decoder.decode(img)
results = recognizer.run(framebuffer)

if results:
    for face in results:
        if face['person']:
            print(f"Recognized {face['person']['name']} (ID: {face['person']['id']})")
            print(f"Similarity: {face['person']['similarity']}")
```

## Benchmark results
The following table shows the frames per second (fps) for different image sizes and models. The results are based on a test with a 2MP camera and a ESP32S3.

| Frame Size  | FaceDetector | HumanDetector |
|-------------|--------------|---------------|
| QQVGA       | 14.5         | 6.6           |
| R128x128    | 21           | 6.6           |
| QCIF        | 19.7         | 6.5           |
| HQVGA       | 18           | 6.3           |
| R240X240    | 16.7         | 6.1           |
| QVGA        | 15.2         | 6.6           |
| CIF         | 13           | 5.5           |
| HVGA        | 11.9         | 5.3           |
| VGA         | 8.2          | 4.4           |
| SVGA        | 6.2          | 3.8           |
| XGA         | 4.1          | 2.8           |
| HD          | 3.6          | 2.6           |

## Notes & Best Practices

1. **Image Format**: Always ensure input images are in the right format. Use mp_jpeg for JPEG decoding from camera.

2. **Memory Management**: 
   - Close/delete detector objects when no longer needed
   - Consider memory constraints when choosing image dimensions

3. **Face Recognition**:
   - Enroll faces in good lighting conditions
   - Multiple enrollments of the same person can improve recognition
   - Use `validate=True` during enrollment to avoid duplicates

4. **Storage**:
   - Face database is persistent across reboots
   - Consider backing up the face database file

