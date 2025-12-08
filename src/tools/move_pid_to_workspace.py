#!/usr/bin/env python3
"""
move_pid_to_workspace.py - Move a window to a specific workspace in Hyprland by PID
The original file is located at ~/.local/bin/move_pid_to_workspace.py
"""


import argparse
import json
import subprocess
import sys
import time
import os
from typing import Optional

VERBOSE = False


def verbose_print(msg: str):
    """Print message only if VERBOSE is True"""
    if VERBOSE:
        print(msg, file=sys.stderr)


def run_command(cmd: list) -> tuple[str, str, int]:
    """Run a shell command and return output, error, and return code"""
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=False)
        verbose_print(f"Command: {' '.join(cmd)}")
        verbose_print(f"Output: {result.stdout[:100]}...")
        verbose_print(f"Return code: {result.returncode}")
        return result.stdout.strip(), result.stderr.strip(), result.returncode
    except Exception as e:
        verbose_print(f"Command exception: {e}")
        return "", str(e), 1


def is_pid_alive(pid: int) -> bool:
    """Check if a process is still running"""
    try:
        os.kill(pid, 0)  # Signal 0 just checks if process exists
        return True
    except (OSError, ProcessLookupError):
        return False


def get_window_by_pid(pid: int) -> Optional[str]:
    """
    Get Hyprland window address by PID.
    Returns window address (hex string like 0x12345678) or None if not found.
    """
    verbose_print(f"Looking for window with PID {pid}")
    stdout, stderr, code = run_command(["hyprctl", "clients", "-j"])

    if code != 0:
        print(f"Error getting clients: {stderr}", file=sys.stderr)
        return None

    try:
        clients = json.loads(stdout)
        for i, client in enumerate(clients):
            verbose_print(f"Client {i}: PID={client.get('pid')}, Address={client.get('address')}, Class={client.get('class')}")

            if client.get("pid") == pid:
                verbose_print(f"Found window by PID match: {client.get('address')}")
                return client.get("address")

    except json.JSONDecodeError as e:
        print(f"Error parsing JSON: {e}", file=sys.stderr)
        return None
    except Exception as e:
        print(f"Error processing clients: {e}", file=sys.stderr)
        return None

    verbose_print("No window found with matching PID")
    return None


def move_window_to_workspace(window_addr: str, workspace: str) -> bool:
    """
    Move a window to a specific workspace.
    workspace can be integer (2) or string with special workspace ("special:magic")
    """
    verbose_print(f"Moving window {window_addr} to workspace {workspace}")

    if not window_addr.startswith("0x"):
        window_addr = f"0x{window_addr}"

    cmd = ["hyprctl", "dispatch", "movetoworkspacesilent", f"{workspace},address:{window_addr}"]
    stdout, stderr, code = run_command(cmd)

    if code != 0:
        print(f"Error moving window: {stderr}", file=sys.stderr)
        return False

    print(f"Moved window {window_addr} to workspace {workspace}")
    return True


def wait_for_window(pid: int, timeout: float = 5.0, poll_interval: float = 0.1) -> Optional[str]:
    """
    Wait for a window to appear for a given PID.
    Returns window address if found, None if timeout.
    """
    verbose_print(f"Waiting for window for PID {pid} (timeout: {timeout}s)")
    start_time = time.time()
    attempts = 0

    while time.time() - start_time < timeout:
        attempts += 1
        verbose_print(f"Attempt {attempts} to find window...")

        if not is_pid_alive(pid):
            print(f"Process {pid} is no longer alive", file=sys.stderr)
            return None

        window_addr = get_window_by_pid(pid)
        if window_addr:
            verbose_print(f"Found window after {attempts} attempts: {window_addr}")
            return window_addr

        time.sleep(poll_interval)

    print(f"Timeout: Window for PID {pid} not found after {timeout} seconds", file=sys.stderr)
    return None


def move_pid_to_workspace(pid: int, workspace: str, timeout: float = 5.0):
    """
    Main function: Move window by PID to workspace.

    Args:
        pid: Process ID of the window
        workspace: Workspace number or special workspace name
        timeout: How long to wait for window to appear (seconds)
    """
    verbose_print(f"Starting move_pid_to_workspace(pid={pid}, workspace={workspace})")

    # First, try to get window by PID
    window_addr = wait_for_window(pid, timeout)

    if not window_addr:
        print(f"Could not find window for PID {pid}", file=sys.stderr)
        sys.exit(1)

    # Move the window
    if not move_window_to_workspace(window_addr, workspace):
        sys.exit(1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Move a window to a specific workspace in Hyprland by PID",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s 12345 2                    # Move PID 12345 to workspace 2
  %(prog)s -p 12345 -w 3              # Move PID 12345 to workspace 3
  %(prog)s -p 12345 -w special:magic  # Move to special workspace
        """,
    )

    # Positional arguments
    parser.add_argument("pid", nargs="?", type=int, help="Process ID of the window")
    parser.add_argument("workspace", nargs="?", type=str, help="Workspace number or name")

    # Optional arguments
    parser.add_argument("-p", "--pid", dest="pid_arg", type=int, help="Process ID")
    parser.add_argument("-w", "--workspace", dest="workspace_arg", type=str, help="Workspace")
    parser.add_argument("-t", "--timeout", type=float, default=5.0, help="Timeout in seconds to wait for window (default: 5.0)")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output for debugging")

    args = parser.parse_args()

    # Set global VERBOSE flag
    VERBOSE = args.verbose

    # Determine which PID/workspace to use
    pid = args.pid_arg if args.pid_arg is not None else args.pid
    workspace = args.workspace_arg if args.workspace_arg is not None else args.workspace

    if pid is None:
        parser.error("PID is required (either positional or -p/--pid)")

    if workspace is None:
        parser.error("Workspace is required (either positional or -w/--workspace)")

    verbose_print(f"Parsed arguments: PID={pid}, Workspace={workspace}, Timeout={args.timeout}s")

    # Check if Hyprland is running
    stdout, stderr, code = run_command(["hyprctl", "version"])
    if code != 0:
        print("Error: Hyprland is not running or hyprctl not available", file=sys.stderr)
        sys.exit(1)

    # Execute the move
    move_pid_to_workspace(pid=pid, workspace=workspace, timeout=args.timeout)
