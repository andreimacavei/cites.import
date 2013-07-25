#!/usr/bin/python

import os
#from os import chdir, path, makedirs
#import shutil

def find_and_copy(pdfs_dir, output_dir, name, doc_number):
    os.chdir(pdfs_dir)
    path_pieces = os.path.split(name)
    pdf_name = path_pieces[-1]
    parent_path = os.path.join(*path_pieces[:-1])
    dest_path = os.path.join(output_dir, parent_path)

    for r,d,files in os.walk(pdfs_dir):
        for f in files:
            if f.lower() == pdf_name.lower():
                file_path = os.path.join(r, f)
                if not os.path.exists(dest_path):
                    os.makedirs(dest_path)
                shutil.copy(file_path, dest_path)

    if not os.path.exists(os.path.join(dest_path, pdf_name)):
        print '{0}: {1}'.format(doc_number, pdf_name)


def copy_files(pdfs_dir, output_dir, names):

    for name in names:
    	path_pieces = os.path.split(name['path'])
        pdf_name = path_pieces[-1]
        parent_path = os.path.join(*path_pieces[:-1])
        pdf_path = os.path.join(pdfs_dir, pdf_name)
        if os.path.isfile(pdf_path):
            dest_path = os.path.join(output_dir, parent_path)
            if not os.path.exists(dest_path):
                os.makedirs(dest_path)

            shutil.copy(pdf_path, dest_path)

if __name__ == '__main__':
    pass
