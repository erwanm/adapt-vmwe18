

rootVMWE=$(pwd)

cd code; make; cd ..

export PATH=$PATH:$rootVMWE/code:$rootVMWE/scripts:$rootVMWE/seq/scripts:$rootVMWE/dep-tree/scripts/:$rootVMWE/shared-task-scripts/


