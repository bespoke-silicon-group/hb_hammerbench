#Process level parallelism for make profile execution
import glob
import sys, os
import subprocess as sp
import multiprocessing as mp
import argparse
from os.path import exists as file_exists  


parser = argparse.ArgumentParser()

parser.add_argument("--dirpattern", type=str, default="wiki-Vote", 
                        help="pattern of the working directories")
args = parser.parse_args()

def check_done(path):
  logfile = path + "profile.log"
  if not file_exists(logfile):
    return False
  with open(logfile) as myfile:
    return ("BSG COSIM PASS: Test passed!" in myfile.read())
  

def work(path):
    #if check_done(path):
    #  print("done: " + path)
    #else:
    #  print("not done: " + path)
    #return 0
    while not check_done(path):
      print("running: " + path)
      sp.run(["make", "clean"],cwd=path,stdout=sp.DEVNULL,stderr=sp.STDOUT)
      sp.run(["make","profile.log"],cwd=path,stdout=sp.DEVNULL,stderr=sp.STDOUT)
    print("complete:", path)
    return 0

if __name__ == '__main__':
    #Specify files to be worked with typical shell syntax and glob module
    
    
    #folder_pattern = '../wiki-Vote*/'
    pattern = args.dirpattern+'*'
    absolute_path = os.path.dirname(__file__)
    relative_path = "../"
    work_path = os.path.join(absolute_path,relative_path,pattern," ")
    work_path = str(os.path.normpath(work_path))
    work_path = work_path.strip()
    tasks = glob.glob(work_path)
    #print(tasks)
    
    #Set up the parallel task pool to use all available processors
    count = mp.cpu_count()
    count_r = min(count,32)
    pool = mp.Pool(processes=count_r)

    #Run the jobs
    pool.map(work, tasks)


    
