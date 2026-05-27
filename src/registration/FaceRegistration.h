// Header file for the face registration process
//
// This file will contain the FaceRegistration class which will do the following:
//  1.Initialise the camera and show a prewview
//  2.Use haar cascade to detect a face in frame
//  3.Take 30 pictures, of the face convert it into greyscale
//  4.Train an LBPH recogniser on those images
//  5.Save the trained modles as .yml files
//
// Libraries needed: OpenCV, Qt(6/5)
