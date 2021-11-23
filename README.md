# Security Camera Facial Recognition
This project uses Dlib and OpenCV to **find** and **recognize** faces within security cameras using FaceNet.

This project still has a downside, the accurate method for clustering the faces  (*[chinese whisperers or CW](https://en.wikipedia.org/wiki/Chinese_Whispers_%28clustering_method%29)*) is not fast enough for real-time, as it needs to be rebuilt each time a new face needs to be labeled.

So I used an area clustering using the smallest enclosing 128 dimensions "circle" in conjunction with CW, so new samples just need to be checked if they are inside this area, but the precision is poor. (probably they aren't classified in a "circular" shape... )

Finer tuning is needed for better accuracy while maintaining good performance,  by finding a better clustering method.

compiling OpenCV and DLIB from sources: [Here](https://gist.github.com/angelorodem/4633f1d04f0a80ba4a195853a179f7f7)
