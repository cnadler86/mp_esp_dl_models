import network
import asyncio
import time
from acamera import Camera, FrameSize, PixelFormat # Import the async version of the Camera class, you can also use the sync version (camera.Camera)
from jpeg import Decoder
import espdl
import json

cam = Camera(fb_count=1, frame_size=FrameSize.VGA, pixel_format=PixelFormat.JPEG, jpeg_quality=85, init=False)
# WLAN config
ssid = 'SSID'
password = 'PWD'
BoxSettings = {'color': {85: "green", 60: "yellow", 0: "red"}}

# Connect to Wi-Fi
station = network.WLAN(network.STA_IF)
station.active(True)
station.connect(ssid, password)

while not station.isconnected():
    time.sleep(1)

print(f'Connected! IP: {station.ifconfig()[0]}. Open this IP in your browser')

try:
    with open("CameraSettings.html", 'r') as file:
        html = file.read()
except Exception as e:
    print("Error reading CameraSettings.html file. You might forgot to copy it from the examples folder.")
    raise e

BB = None
Model = None

def LookupTable(value, dictionary):
    for key in sorted(dictionary.keys(), reverse=True):
        if value >= key:
            return dictionary[key]
    return None

async def stream_camera(writer):
    global BB
    global Model
    try:
        cam.init()
        cam.set_vflip(True)
        Model = espdl.FaceDetector(width=cam.get_pixel_width(), height=cam.get_pixel_height())
        Dec = Decoder()
        await asyncio.sleep(1)
        
        writer.write(b'HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n')
        await writer.drain()

        while True:
            frame = await cam.acapture() # This is the async version of capture, you can also use frame = cam.capture() instead
            if frame:
                writer.write(b'--frame\r\nContent-Type: image/jpeg\r\n\r\n')
                writer.write(frame)
                BB = Model.run(Dec.decode(frame))
                await writer.drain()
                
    finally:
        cam.deinit()
        writer.close()
        await writer.wait_closed()
        print("Streaming stopped and camera deinitialized.")

async def handle_client(reader, writer):
    global Model
    try:
        request = await reader.read(1024)
        request = request.decode()

        if 'GET /stream' in request:
            print("Start streaming...")
            await stream_camera(writer)
        elif 'GET /get_boxes' in request:
            bounding_boxes = []
            if BB is not None:
                for box in BB:
                    color = LookupTable(box['score']*100, BoxSettings['color'])
                    Dict = {"x1":box['box'][0],
                            "y1":box['box'][1],
                            "x2":box['box'][2],
                            "y2":box['box'][3],
                            "label":f"Score: {box['score']*100:.0f}%",
                            "color":color}
                    if Model.__class__.__name__ == "FaceRecognizer":
                        if box['person'] is not None:   
                            Dict['label'] = f"{box['person']['name']} ({box['person']['similarity']*100:.0f}%), Score: {box['score']*100:.0f}%"
                    bounding_boxes.append(Dict)
            response = 'HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n' + json.dumps(bounding_boxes)
            writer.write(response.encode())
            await writer.drain()
        elif 'GET /set_' in request:
            method_name = request.split('GET /set_')[1].split('?')[0]
            if method_name == "model":
                model_name = request.split('value=')[1].split(' ')[0]
                ModelClass = getattr(espdl, model_name)
                Model = ModelClass(width=cam.get_pixel_width(), height=cam.get_pixel_height())
                response = 'HTTP/1.1 200 OK\r\n\r\n'
                writer.write(response.encode())
                await writer.drain()
                return
            value = int(request.split('value=')[1].split(' ')[0])
            set_method = getattr(cam, f'set_{method_name}', None)
            if callable(set_method):
                set_method(value)
                print(f"{method_name} setted to {value}")
                if method_name == 'frame_size':
                    Model.width=cam.get_pixel_width()
                    Model.height=cam.get_pixel_height()
                response = 'HTTP/1.1 200 OK\r\n\r\n'
                writer.write(response.encode())
                await writer.drain()
            else:
                try:
                    cam.reconfigure(**{method_name: value})
                    print(f"Camera reconfigured with {method_name}={value}")
                    print("This action restores all previous configuration!")
                    response = 'HTTP/1.1 200 OK\r\n\r\n'
                except Exception as e:
                    print(f"Error with {method_name}: {e}")
                    response = 'HTTP/1.1 404 Not Found\r\n\r\n'
                writer.write(response.encode())
                await writer.drain()

        elif 'GET /get_' in request:
            method_name = request.split('GET /get_')[1].split(' ')[0]
            get_method = getattr(cam, f'get_{method_name}', None)
            if callable(get_method):
                value = get_method()
                response = f'HTTP/1.1 200 OK\r\n\r\n{value}'
                writer.write(response.encode())
                await writer.drain()
            else:
                if method_name == "model" and Model is not None:
                    value = Model.__class__.__name__
                    response = f'HTTP/1.1 200 OK\r\n\r\n{value}'
                    pass
                else:
                    response = 'HTTP/1.1 404 Not Found\r\n\r\n'
                writer.write(response.encode())
                await writer.drain()
            print(f"{method_name} is {value}")

        else:
            writer.write('HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n'.encode() + html.encode())
            await writer.drain()
    except Exception as e:
        print(f"Error: {e}")
    finally:
        writer.close()
        await writer.wait_closed()

async def start_server():
    server = await asyncio.start_server(handle_client, "0.0.0.0", 80)
    ip_address = station.ifconfig()[0]
    print(f'Server is running on http://{ip_address}')
    
    while True:
        await asyncio.sleep(3600)

try:
    asyncio.run(start_server())
except KeyboardInterrupt:
    cam.deinit()
    print("Server stopped")
