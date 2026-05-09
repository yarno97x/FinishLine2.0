# Formula 1 Race Predictor

Personal project focused on building an end-to-end machine learning workflow for Formula 1 race prediction, from data collection to model inference and visualization.

## Overview

This project combines C++ data engineering and Python deep learning to predict race outcomes from historical F1 data.

- Built a C++ preprocessing pipeline to scrape, clean, and merge 75 seasons of Formula 1 data.
- Trained temporal convolutional neural networks (CNNs) in PyTorch on driver, constructor, track, and session-history features.
- Designed a Qt desktop interface for dataset exploration and prediction confidence visualization.
- Structured the codebase with modular C++ and Python components for model upgrades and automated data refresh workflows.ndling and scaling utilities with unit tests.
- Web scraping utilities for race and season data collection.
- Experiment-friendly architecture for rapid feature/model iteration.

## Tech Stack

- C++ (core data pipeline and transform system)
- PyTorch (temporal CNN training and evaluation)
- Qt (desktop UI for exploration and prediction display)
- CMake + vcpkg (build and dependency management)
- GoogleTest (unit testing)

## Build (C++ Pipeline)

Prerequisites:
- CMake
- A C++ compiler with C++17+ support
- vcpkg (if you use manifest mode dependencies)

```bash
cmake -S . -B build
cmake --build build
```

## Coverage (gcov)

Run tests and generate `gcov` reports:

```bash
./build.sh coverage
```

Generated coverage files are written to `coverage/`.

## Project Goals

- Improve race outcome prediction quality with richer temporal features.
- Expand model comparisons beyond temporal CNN baselines.
- Add automated refresh pipelines for new race weekends.
- Improve UI explainability around confidence, uncertainty, and feature impact.
