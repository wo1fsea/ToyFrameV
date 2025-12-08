# ToyFrameV - Running Examples with Virtual Framebuffer

When running ToyFrameV example programs in headless environments (such as Docker containers, CI/CD pipelines), you need to use Xvfb (X Virtual Framebuffer).

## Installing Xvfb

```bash
sudo apt-get update
sudo apt-get install -y xvfb
```

## Method 1: Using Helper Script

The project provides a convenient script to automatically manage Xvfb:

```bash
# Run HelloApp
./scripts/run_with_xvfb.sh ./build/bin/HelloApp

# Run HelloTriangle
./scripts/run_with_xvfb.sh ./build/bin/HelloTriangle

# Run HelloIO
./scripts/run_with_xvfb.sh ./build/bin/HelloIO
```

## Method 2: Manual Xvfb Setup

```bash
# 1. Start Xvfb (only need to start once)
Xvfb :99 -screen 0 1280x720x24 &

# 2. Set DISPLAY environment variable and run program
DISPLAY=:99 ./build/bin/HelloApp

# Or export DISPLAY variable
export DISPLAY=:99
./build/bin/HelloApp
./build/bin/HelloTriangle
```

## Stopping Xvfb

```bash
pkill Xvfb
```

## Common Issues

### Program Crashes or Shows "Failed to open X display"

Make sure:
1. Xvfb is running: `ps aux | grep Xvfb`
2. DISPLAY variable is set: `echo $DISPLAY`

### Check if Xvfb is Running

```bash
ps aux | grep Xvfb | grep -v grep
```

### View Xvfb Logs

For debugging, you can run Xvfb without redirecting output:

```bash
Xvfb :99 -screen 0 1280x720x24
```

## Advanced Configuration

### Using Different Display Number

```bash
# Use display :100
Xvfb :100 -screen 0 1280x720x24 &
DISPLAY=:100 ./build/bin/HelloApp
```

### Configure Different Resolution and Color Depth

```bash
# 1920x1080, 32-bit color
Xvfb :99 -screen 0 1920x1080x32 &
DISPLAY=:99 ./build/bin/HelloApp
```

### Multi-Monitor Configuration

```bash
Xvfb :99 -screen 0 1280x720x24 -screen 1 1920x1080x24 &
```

## Using in CI/CD

### GitHub Actions Example

```yaml
- name: Install Xvfb
  run: sudo apt-get install -y xvfb

- name: Run tests with Xvfb
  run: |
    Xvfb :99 -screen 0 1280x720x24 &
    export DISPLAY=:99
    ./build/bin/HelloApp
```

### Docker Example

```dockerfile
# In Dockerfile
RUN apt-get update && apt-get install -y xvfb

# At runtime
CMD Xvfb :99 -screen 0 1280x720x24 & \
    export DISPLAY=:99 && \
    ./build/bin/HelloApp
```
