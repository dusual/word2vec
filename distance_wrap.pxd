cdef extern from "distance.h":
    cdef struct model_data:
        float *M
        char *vocab
        char *bestw[10]
        float bestd[10]
        long long words, size
        int a, b
        float len, vec[2000]

    float findDistance(model_data *, char *, char *)
