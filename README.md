# NTHU Computer Graphics Assignment 1

## Usage
```bash
make run
# or
make cleanrun # clean and run
```

## Notes

### Frame Buffer Size
* HiDpi causes frame buffer scaled up, making it different from the specified window size.
* Solve by using frame buffer size instead of window size.
```cpp
int frame_width, frame_height;
glfwGetFramebufferSize(glfwGetCurrentContext(), &frame_width, &frame_height);
glViewport(i * window_width / 2.0f, 0, frame_width / 2.0f, frame_height);
```
