from root_converter import NGMRootFile
import argparse

if __name__ == '__main__':

    parser = argparse.ArgumentParser('Convert root file into Pandas data frame (optionally write an .h5 file)')
    parser.add_argument('file', help='Root file')
    parser.add_argument("--white_file", "-w", action="store_true", help="write an .h5 file containing the data")
    _args = parser.parse_args()

    converter = NGMRootFile(_args.file)
