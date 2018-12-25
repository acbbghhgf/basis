#config

face_cascade = "haarcascades/haarcascade_frontalface_alt2.xml"

img_width = 119
img_height = 146

img_kinds = ['disgust', 'fear', 'happiness', 'others', 'repression', 'surprise']

kernel_size = 21

kernel_var = 5
gabor_psi = 90

gabor_phase = [0, 30, 60, 90, 120, 150, 180]
gabor_pulsation = [2, 4, 6, 8, 10]

train_subdir = 'data/train/'
test_subdir = "data/test/"
face_subdir = "data/face/"
original_subdir = "data/original/"
models_dir = "data/models/"
svm_params = "-t 0 -c 3"

pca_components = 30

lda_components = 2