# ESP DL MicroPython Binding

This is a micropython binding for the esp dl models `FaceDetector`, `ImageNet`, and `PedestrianDetector`.

## Precompiled images

You may find precompiled images in the actions section for passed workflows under artifacts. You can also fork the repo and start the action manually in order to build the library.

## Importing Modules

To use any of the modules, you need to import them first. Here is how you can import each module:

```python
from espdl import FaceDetector
from espdl import FaceRecognizer
from espdl import PedestrianDetector
from espdl import ImageNet
```

## FaceDetector Module

To create an instance of the `FaceDetector`:

```python
face_detector = FaceDetector(width=320, height=240, features=True)
```

### FaceDetector Methods

- **Detect Faces**

  ```python
  results = face_detector.run(framebuffer)
  ```

  - `framebuffer`: The image data to be processed in RGB888 format.

## FaceREcognizer Module

To create an instance of the `FaceDetector`:

```python
FR = FaceRecognizer(width=320, height=240, db_path="MyFaces.db", )
```

`db_path` is optional.

### Enroll a face (save it to the DB)
```python
RF.enroll(Img)
```

### Recognize a face
```python
FaceIDandSimilarity = RF.run(Img)
```

## PedestrianDetector Module

To create an instance of the `PedestrianDetector`:

```python
pedestrian_detector = PedestrianDetector(width=320, height=240)
```

### PedestrianDetector Methods

- **Detect Pedestrians**

  ```python
  results = pedestrian_detector.run(framebuffer)
  ```

  - `framebuffer`: The image data to be processed.

## ImageNet Module

To create an instance of the `ImageNet`:

```python
image_net = ImageNet(width=320, height=240)
```

### ImageNet Methods

- **Classify Image**

  ```python
  results = image_net.run(framebuffer)
  ```

  - `framebuffer`: The image data to be processed in RGB888 format.

## Example Usage

Here is an example of how to use the `FaceDetector` module:

```python
from espdl import FaceDetector

# Create an instance of FaceDetector with defaults width=320, height=240, features=True
face_detector = FaceDetector()

# Assume framebuffer is obtained from the camera and decoded with mp_jpeg to RGB888
framebuffer = decoder.decode(cam.capture())

# Detect faces
results = face_detector.run(framebuffer)

# Process results
if results:
    for result in results:
        print("Score:", result[0])
        print("Bounding Box:", result[1])
        if len(result) > 2:
            print("Features:", result[2])
```

- The `results` data for face detection includes the detection score and bounding box coordinates `[x1, y1, x2, y2]`.
- If `features` are enabled, the `results` also include keypoints for facial features in x-y coordinates: left eye, left mouth, nose, right eye, right mouth.
Similarly, you can use the `ImageNet` and `PedestrianDetector` modules by following the same pattern.
`ImageNet` returns the identified classes and respective scores, `PedestrianDetector` only score and bounding box.

## Notes

- Ensure that the `framebuffer` contains valid image data in RGB888 format before calling the `detect` method. Take a look at [mp_jpeg](https://github.com/cnadler86/mp_jpeg/) for this purpose.
- The `width` and `height` parameters should match the dimensions of the image data in the `framebuffer`.

## Building the Firmware

1. **Clone the necessary repositories:**

    ```sh
    git clone https://github.com/cnadler86/mp_esp_dl_models.git
    git clone https://github.com/cnadler86/micropython-camera-API.git
    git clone https://github.com/cnadler86/mp_jpeg.git
    ```

2. **Build the firmware:**

    Be sure to have the whole esp32 build environment for micropython available.
    Please populate your placeholders with the desired board. Starting folder is the git clone of this repo.

    ```sh
    cd boards/
    idf.py -D MICROPY_DIR=<micropython-dir> -D MICROPY_BOARD=<BOARD_NAME> -D MICROPY_BOARD_VARIANT=<BOARD_VARIANT> -B build-<your-build-name> build
    cd build-<your-build-name>
    python ~/micropython/ports/esp32/makeimg.py sdkconfig bootloader/bootloader.bin partition_table/partition-table.bin micropython.bin firmware.bin micropython.uf2
    ```
