import os
import logging
import pdb
import re
import subprocess

def remove_comments(c_code):
    # Regular expression to match both single-line and multi-line comments
    pattern = r"//.*?$|/\*.*?\*/"
    
    # Substitute the comments with an empty string
    cleaned_code = re.sub(pattern, '', c_code, flags=re.DOTALL | re.MULTILINE)
    
    return cleaned_code.strip()

def sanitize_files(path: str):
    for root, directories, files in os.walk(path): 
        for filename in files:
            if filename.endswith(".c") and "sanitized" not in filename:
                filepath = os.path.join(root, filename)

                # os.remove(filepath)
                
                with open(filepath, "r") as f:
                    c_code = f.read()
                c_code = remove_comments(c_code)
                if ".res.c" not in filename:
                    filepath = os.path.join(root, filename + ".sanitized.c")
                else:
                    filepath = os.path.join(root, filename + ".sanitized.res.c")
                with open(filepath, "w") as f:
                    f.write(c_code)
                logging.info(f"Sanitize {filepath}")
                subprocess.run(["clang-format", "-i", filepath], check=True)
    

if __name__ == "__main__":
    try:
        PATH = (
            "/media/hdd2/yusuf/SMPL-How-Far-Are-We/emse_submission_fix/k_1/inference",
            "/media/hdd2/yusuf/SMPL-How-Far-Are-We/emse_submission_fix/k_2/inference",
            "/media/hdd2/yusuf/SMPL-How-Far-Are-We/emse_submission_fix/k_3/inference",
        )
        for path in PATH:
            sanitize_files(path)
    except Exception as e:
        logging.error(e)
        pdb.post_mortem()
