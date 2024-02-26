# stm32f7-Pedalboard

Greetings, within this repository lies the culmination of our Electronic Engineering thesis project. It encompasses a pedalboard designed to apply 12 distinct popular audio effects to a single-ended input, achieving a sampling rate of 44.1 kHz with a depth of 24 bits. The primary aim was to craft a high-quality sounding platform for conveniently testing applied effects. The software architecture is designed for effortless addition of new effects, fostering a user-friendly environment for development and learning. Additionally, the hardware utilized is a readily accessible development kit, widely available in numerous countries.

Here you can find a functional demo:

<iframe width="560" height="315" src="https://www.youtube.com/embed/V-5spfllwe8?si=3IBq79IgztJ8RXuU" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" allowfullscreen></iframe>

### Quick Links

- [Getting Started](#getting-started)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Roadmap](#roadmap)
- [License](#license)

## Getting Started

The demonstration above illustrates the collaboration of various components to achieve the desired functionality.

Key components include:

- Touch screen: Utilized for navigating the user interface (UI), selecting effects, and toggling functionalities.
- Audio codec: Provided by the kit, responsible for sampling input and generating output waveforms (ADC/DAC).
- SD storage: Stores UI images and recorded audio.
- Micro-controller: Serves as the central processing unit, handling computations for effects, coordinating external devices, and managing internal peripherals. 

For detailed information, refer to this [document](https://drive.google.com/file/d/16MncO3Z1pbzFfrqieCRYZ0KL5QBh5DHn/view?usp=sharing), which covers software, hardware, and mathematical aspects of the project.

### Implemented Effects:

- Filter-based Effects:
    - Equalizer
    - Auto-Wah
    - Phaser
- Delay-based Effects:
    - Delay
    - Echo
    - Delay + Echo
    - Vibrato
    - Chorus
    - Flanger
    - Reverb
    - Octave
- Modulation-based Effects:
    - Tremolo
    - Ringmod
- Non-linear Effects:
    - Distortion

### Hardware Information::

Discovery kit [STM32F746G](https://www.st.com/en/evaluation-tools/32f746gdiscovery.html):
- Cortex M7, 1Mb flash and 340Kb RAM
- Audio codec [WM8994](https://www.cirrus.com/products/wm8994/)
- 128 Mb Quad-SPI NOR flash
- 128 Mb SDRAM
- 4.3” RGB 480×272 color LCD-TFT with capacitive touch screen
- On-board ST-LINK/V2-1 debugger/programmer with USB re-enumeration capability: mass storage, Virtual COM port, and debug port

### Software parts

- GUI: Manages the UI, controlling the rendering of knobs, LEDs, and buttons. UI elements' positions are based on the 480×272 display resolution.
- Effects: All effects code files are on the same directory and are explained in the [document](https://drive.google.com/file/d/16MncO3Z1pbzFfrqieCRYZ0KL5QBh5DHn/view?usp=sharing) with equations and diagrams.
- Finite-state machine: Renders the UI, applies effects to audio buffers, and records audio based on the state value. 

### References:

Various sources including textbooks and papers on digital signal processing and audio effects are utilized in this project.

To list a few:

- Smith, Steven W. “The Scientist and Engineer's Guide to Digital Signal Processing”, 1997.
- Smith, J.O. “Physical Audio Signal Processing”, 2010.
- Zölzer, Udo “DAFX: Digital Audio Effects”, 2002.
- Zölzer, Udo “Digital Audio Signal Processing”, 2da Edition, 2008.
- Dodge, Charles & Jerse, Thomas A. “Computer Music: Synthesis, Composition and
Performance”, 1997.
- Ofrandis, S.J. “Introduction to Signal Processing”, 1996.
- Yeh, D.T. & Abel, J & Smith J.O, “Simulation of the Diode Limiter in Guitar Distortion
Circuits by Numerical Solution of Ordinary Differential Equations”, 2007.
- Dattorro, J. “Effect Design”, 1997.
- Zölzer, Udo & Holters, Martin “Parametric Higher-Order Shleving Filters”, 2006.
- Cohen, Ivan, “Fifty Shades of Distortion”, 2017.

### Prerequisites

To compile the project, install [MDK-ARM](https://www.keil.com/demo/eval/arm.htm) from Keil as this is a uVision project. Alternatively, consider using the new [MDK v6 Community Edition](https://www.keil.arm.com/community/) with [VsCode](https://code.visualstudio.com/download). Refactoring the project to not require a paid IDE is planned.

### Installation

`git clone https://github.com/codewarlocks/stm32f7-Pedalboard.git`

## Usage

To run the project, save all contents of the assets folder to the root directory of an SD card (ensure 'Media' and 'Perillas' folders are in the root directory) and insert it into the kit. Then, execute the program with uVision.

Usage is straightforward with three menus: two for pedals and one for the record feature. Pedals can be activated by pressing the switch button or the pedal itself to enter an individual view for tuning parameters. Navigation between pedals is done using left/right arrows or returning to the menu.

For connecting inputs/outputs, replace the provided jack in the kit with 1/4 mono jacks.

## Roadmap

- [ ] Enhance documentation
- [ ] Improve error handling
- [ ] Add more effects
- [ ] Refactor recording feature
- [ ] Refactor project to support a non commercial IDE as Vscode
- [ ] Improve audio quality (reduce noise, increase sampling rate, etc)
- [ ] Add external components as switches or buttons
- [ ] Add battery support
- [ ] Design a suitable replacement to the development kit
- [ ] Make software more platform agnostic
- [ ] Refactor audio effects as a library in Rust

## License
