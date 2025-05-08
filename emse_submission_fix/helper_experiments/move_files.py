import os
import shutil
import fnmatch
import re

def copy_files(source_path, target_path):
    """
    Copy specific files from source_path to target_path based on criteria:
    1. Copy all .c and .res.c files that don't contain certain keywords
    2. Copy all .cocci files
    Maintains directory structure in the target path.
    
    Args:
        source_path (str): Source directory path
        target_path (str): Target directory path
    """
    # Make sure target directory exists
    os.makedirs(target_path, exist_ok=True)
    
    # Define file patterns to match
    c_patterns = ['*.c', '*.res.c']
    excluded_keywords = ['gpt', 'deepseek', 'spinfer', 'claude', 'sanitized']
    cocci_pattern = '*.cocci'
    
    # Create a regex pattern for excluded keywords
    excluded_regex = re.compile('|'.join(excluded_keywords), re.IGNORECASE)
    count_c = 0
    count_cocci = 0
    # Walk through the source directory
    for root, dirs, files in os.walk(source_path):
        
        # Process files in the current directory
        for filename in files:            
            # Check if it's a .c or .res.c file without excluded keywords
            if (any(fnmatch.fnmatch(filename, pattern) for pattern in c_patterns) and not excluded_regex.search(filename)) or fnmatch.fnmatch(filename, cocci_pattern):
                # Get relative path to maintain directory structure
                rel_path = os.path.relpath(root, source_path)
                source_file = os.path.join(root, filename)
                
                # Create target directory if needed
                target_dir = os.path.join(target_path, rel_path)
                os.makedirs(target_dir, exist_ok=True)
                
                # Copy the file
                target_file = os.path.join(target_dir, filename)
                shutil.copy2(source_file, target_file)
                if any(fnmatch.fnmatch(filename, pattern) for pattern in c_patterns) and not excluded_regex.search(filename):
                    count_c += 1

                if fnmatch.fnmatch(filename, cocci_pattern):
                    count_cocci += 1

                print(f"Copied file: {source_file} -> {target_file}")
    print(f"Total copied C files: {count_c}")
    print(f"Total copied Cocci files: {count_cocci}")

# Example usage
if __name__ == "__main__":
    # Replace with your actual source and target paths
    path_source = [
        "/media/hdd2/yusuf/smpl/emse_submission/fix_bug/3_ex"
    ]
    path_target = [
        "/media/hdd2/yusuf/smpl/emse_submission_fix/k_3"
    ]
    for source, target in zip(path_source, path_target):
        copy_files(source, target)