# Wafer Defect Detection

This project aims to identify the defects in wafer images using computer vision
techniques. The dataset consists of high-resolution images of wafers, which may
or may not contain defects of different types.

The goal is to segment the defective regions in the images and classify the 
type of defect present.

## Setup:

1. Clone the repository to your local machine.
```
git clone https://github.com/vetrivln/WaferDefectDetection
```

2. Add OpenCV to your project:
- Include directories -> `opencv\build\include`
- Library directories -> `opencv\build\x64\vc16\lib`
- Link .lib files -> `opencv_world4xx.lib`

3. Build and Run the project in Visual Studio.


## Requirements:
- **OS:** Windows 10/11 (64 bit)
- **Compiler:** Visual Studio 2019/2022
- **OpenCV:** 4.x
- **.NET:** 4.7.2 or higher