### Define variables

## SSH
UW=linux.student.cs.uwaterloo.ca
USER=jpadilla

# Remote
REMOTE_DIR="~/Documents/courses/cs652/code/microkernel"
REMOTE_SCRIPT_DIR="~/Documents/courses/cs652/code/"

### Execution
# Copies the current folder to the "mounted" folder
echo "Copying files..."
scp -r `ls -1 . | grep -vE '.git|.sdf|.suo|.sln'` $USER@$UW:$REMOTE_DIR ## NOTE: This copies only files that are listed in the .git file. 

# Execute the compilation in the remote machine. 
echo "Compiling..."
ssh $USER@$UW "cd $REMOTE_DIR; cd ..; chmod -R +rwx microkernel/; cd $REMOTE_DIR; pwd; echo \$PATH; make clean; make; make install"

echo "Finished successfully."
