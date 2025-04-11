import os
from collections import Counter

def count_cocci_files(source_path):
    """
    Counts occurrences of .cocci files with the same filename,
    regardless of their directory location.
    
    Args:
        source_path (str): The path to search for .cocci files
        
    Returns:
        None: Prints the counts of each .cocci filename
    """
    if not os.path.exists(source_path):
        print(f"Error: The path '{source_path}' does not exist.")
        return
    
    # Initialize a counter to store filename occurrences
    filename_counter = Counter()
    
    # Keep track of all file paths for each filename for detailed output
    filename_paths = {}
    
    # Walk through the directory tree recursively
    for root, _, files in os.walk(source_path):
        # Filter for .cocci files and count them
        for file in files:
            if file.endswith('.cocci'):
                filename_counter[file] += 1
                
                # Store the full path for this file
                if file not in filename_paths:
                    filename_paths[file] = []
                filename_paths[file].append(os.path.join(root, file))
    
    # Print results
    if not filename_counter:
        print(f"No .cocci files found in '{source_path}'.")
        return
    
    total_files = sum(filename_counter.values())
    unique_filenames = len(filename_counter)
    
    print(f"Found {total_files} .cocci files with {unique_filenames} unique filenames.")
    print("\nCounts per filename:")
    print("-" * 50)
    print(f"{'Filename':<40} {'Count':>10}")
    print("-" * 50)
    
    # Sort by count (descending)
    for filename, count in sorted(filename_counter.items(), key=lambda x: (-x[1], x[0])):
        print(f"{filename:<40} {count:>10}")
        
        # Optionally, show all paths where this file appears
        if count > 1:
            # print(f"\nLocations where '{filename}' appears:")
            # for path in filename_paths[filename]:
            #     print(f"  - {path}")
            print()

# Example usage
if __name__ == "__main__":
    path = "/media/hdd2/yusuf/SMPL-How-Far-Are-We/emse_submission_fix/k_1/inference"
    count_cocci_files(path)