# CGPIO
# Program 8.1 - Shadow Mapping
One of the most practical and popular methods for casting shadows is called shadow mapping. Although it is not always as accurate as shadow volumes (and is often accompanied by pesky artifacts), shadow mapping is easier to implement, can be used in a wide variety of situations, and enjoys powerful hardware support. 

Shadow mapping is based on a very simple and clever idea: anything that cannot be seen by the light is in shadow. This is powerful because we already have a method for determining if something can be "seen" through the hidden surface removal algorithm (HSR) using the Z-buffer.
