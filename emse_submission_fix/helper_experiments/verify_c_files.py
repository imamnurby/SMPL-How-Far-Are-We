import os
import fnmatch
import sys

def check_c_files_exist(reference_path, current_path):
    """
    Check if all C files in current_path exist in reference_path.
    
    Args:
        reference_path (str): Path to the reference directory
        current_path (str): Path to the current directory to check
        
    Returns:
        bool: True if all C files in current_path exist in reference_path, False otherwise
    """
    # Collect all C files recursively in both paths
    reference_c_files = []
    current_c_files = []
    
    # Collect reference C files
    for root, _, files in os.walk(reference_path):
        for file in fnmatch.filter(files, "*.c"):
            if  "sanitized" not in file:
                relative_path = os.path.relpath(os.path.join(root, file), reference_path)
                reference_c_files.append(relative_path)
    
    # Collect current C files
    for root, _, files in os.walk(current_path):
        for file in fnmatch.filter(files, "*.c"):
            if  "sanitized" not in file:
                relative_path = os.path.relpath(os.path.join(root, file), current_path)
                current_c_files.append(relative_path)
    
    # Check if all current C files exist in reference path
    missing_files = []
    for file in current_c_files:
        if file not in reference_c_files:
            missing_files.append(file)
    
    # Print results
    if missing_files:
        print(f"Error: {len(missing_files)} C files in current path not found in reference path:")
        for file in missing_files:
            print(f"  - {file}")
        return False
    else:
        print(f"Success: All {len(current_c_files)} C files in current path exist in reference path ({len(reference_c_files)} files).")
        return True

if __name__ == "__main__":
    reference_path = "/media/hdd2/yusuf/SMPL-How-Far-Are-We/emse_submission_fix/k_3/inference"
    current_path = "/media/hdd2/yusuf/SMPL-How-Far-Are-We/emse_submission_fix/k_2/inference"
    
    check_c_files_exist(reference_path, current_path)