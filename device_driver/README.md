# Character Device Driver - Operating Systems Assignment 1

## Overview

A Linux kernel module that implements a character device driver with timed, ordered read/write operations and kernel version verification.

---

## Prerequisites

- Linux system with kernel headers installed
- `gcc`, `make`, and kernel build tools
- Sudo privileges for module operations

Install kernel headers (if not already installed):

```bash
# Debian/Ubuntu
sudo apt install linux-headers-$(uname -r) build-essential
```
---
## Compiling the files
- For this use make command and use provided Makefile
- Many files will be visible after executing make command ,for this assignment `.ko` file is needed
--- 

## Usage

### 1. Find Your Kernel Version

```bash
uname -r
```

Example output: `6.8.0-45-generic` → major = `6`, minor = `8`

### 2. Insert the Module

```bash
sudo insmod kernel_module.ko kernel_version=<major>,<minor> timer=<seconds>
```

**Example:**

```bash
sudo insmod kernel_module.ko kernel_version=6,8 timer=30
```

**Parameters:**

| Parameter        | Type    | Description                                      |
|------------------|---------|--------------------------------------------------|
| `kernel_version` | int[]   | Two-element array: `major,minor` of your kernel  |
| `timer`          | int     | Time limit in seconds to complete the operations |

### 3. Verify Insertion

```bash
sudo dmesg
```

Expected output:
```
Kernel version matched.
Major number: <X>
Minor number: <Y>
Timer started for: <N> seconds
```

### 4. Create the Device Node

Get the major number from `dmesg` output above, then:

```bash
sudo mknod /dev/mydevice c <major_number> 0
sudo chmod 666 /dev/mydevice
```

### 5. Perform the Required Actions (in order, within the timer)

**Step 1 — Read from the device** (do this first):

```bash
cat /dev/mydevice 
```
**Step 2 — Write the username to the device:**

```bash
echo $USER > /dev/mydevice
```

### 6. Remove the Module

```bash
sudo rmmod kernel_module
```

Then check the result:

```bash
sudo dmesg
```

---

## Expected Outcomes

### Success Case

If read was called before write, and both were completed within the timer:

```
Successfully completed the actions within time.
Username recorded: <username>
Module removed
```

### Failure Cases

| Scenario                        | Kernel Message                                                            |
|---------------------------------|---------------------------------------------------------------------------|
| Wrong kernel version given      | `Kernel Version mismatch!`                                                |
| Fewer than 2 version arguments  | `Insufficient arguments`                                                  |
| Write called before Read        | `Order violation! Write called before Read.`                              |
| Timer expired                   | `Timer expired.`                                                          |
| Actions not completed in order/time | `Failure - Actions were not completed in order within the given time.`|

---

## How It Works

1. **Module Init (`insmod`)**: Verifies the provided kernel version against `LINUX_VERSION_CODE`. On success, allocates a character device region, initializes a wait queue, and starts a jiffies-based countdown timer.

2. **Read (`cat`)**: Checks the timer, marks `read_done = true`, then blocks on a wait queue (`wait_event_interruptible_timeout`) until a write occurs or the timer expires.

3. **Write (`echo`)**: Checks the timer and verifies that read was called first (`order_correct`). Copies the username from userspace into a kernel buffer, sets `write_done = true`, and wakes up the waiting read via `wake_up_interruptible`.

4. **Module Exit (`rmmod`)**: Checks all flags (`read_done`, `write_done`, `order_correct`, `!time_expired`) and prints success or failure accordingly. Cleans up the cdev and device region.

---

## Cleanup

```bash
# Remove device node
sudo rm /dev/mydevice

# Clean built files
make clean
```
