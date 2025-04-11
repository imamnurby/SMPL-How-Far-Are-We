import os
import shutil

def standardize_cocci_files(source):
    """
    Finds all files ending with .cocci in the source directory
    and renames them to 'final_response_0.spinfer.cocci'.
    
    Args:
        source (str): Path to the source directory
    
    Returns:
        int: Number of files processed
    """
    # Counter for processed files
    count = 0
    
    # Check if source directory exists
    if not os.path.isdir(source):
        print(f"Error: {source} is not a valid directory")
        return count
    
    # Walk through the directory structure
    for root, _, files in os.walk(source):
        for file in files:
            # Check if file ends with .cocci
            if file.endswith('.spinfer.cocci'):
                # Construct file paths
                original_path = os.path.join(root, file)
                new_path = os.path.join(root, "final_response_0.spinfer.cocci")
                
                # Rename the file
                shutil.copy2(original_path, new_path)
                os.remove(original_path)
                
                print(f"Renamed: {original_path} -> {new_path}")
                count += 1
    
    print(f"Total .cocci files processed: {count}")
    return count

if __name__ == "__main__":
    standardize_cocci_files("/media/hdd2/yusuf/smpl/emse_submission_fix/k_2/inference")