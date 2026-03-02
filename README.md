# Wafer Defect Detection

[![](https://images2.imgbox.com/d0/f0/iXsAjyM9_o.png)]()

<sub>Detection output on a sample optical lens image: defective regions 
are segmented and highlighted, with defect classification overlaid</sub>

A C++ and OpenCV based computer vision system for automated detection and 
classification of defects on semiconductor wafer surfaces. Designed to segment 
defective regions from high-resolution wafer imagery and identify defect type. 

Built as a proof-of-concept for inline inspection in semiconductor manufacturing workflows

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