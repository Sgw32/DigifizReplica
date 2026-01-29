import gzip
import os
import shutil
import sys


def main():
    src_dir = sys.argv[1]
    dist_dir = sys.argv[2]
    files = sys.argv[3:]

    os.makedirs(dist_dir, exist_ok=True)

    for file in files:
        src_path = os.path.join(src_dir, file)
        dest_path = os.path.join(dist_dir, f"{file}.gz" if not file.endswith('.png') else file)

        print(f"Processing {file} -> {dest_path}")

        if file.endswith('.png'):
            shutil.copy(src_path, dest_path)
        else:
            with open(src_path, 'rb') as f_in:
                with gzip.open(dest_path, 'wb', compresslevel=9) as f_out:
                    shutil.copyfileobj(f_in, f_out)


if __name__ == "__main__":
    main()
