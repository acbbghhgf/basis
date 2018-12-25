import cv
import svm
import svmutil
import glob
import operator
import os
import os.path
import numpy
import pickle
import math
import thread
import sys
from sklearn.lda import LDA
from sklearn.decomposition import PCA
from config import *

models = {}

pca = {}

lda = {}

def get_features(img, height = 8, width = 6):
	features = []

	step_x = img.height / height;
	step_y = img.width / width;

	for x in xrange(0 + step_x, img.height, step_x):
		for y in xrange(0 + step_y, img.width, step_y):
			features.append(img[x, y])
	
	return features


def get_image_features(img):
	features = []

	for phase in gabor_phase:
		for pulsation in gabor_pulsation:
			(t_img_mag, t_img) = gabor(img, kernel_var, pulsation, phase, gabor_psi)
			features.extend(get_features(t_img_mag))

	return features

def gabor(image, pos_var, pos_w, pos_phase, pos_psi):
	global kernel_size
	if kernel_size % 2 == 0:
		kernel_size += 1

	kernel = cv.CreateMat(kernel_size, kernel_size, cv.CV_32FC1)

	src = cv.CreateImage((image.width, image.height), cv.IPL_DEPTH_8U, 1)
	src_f = cv.CreateImage((image.width, image.height), cv.IPL_DEPTH_32F, 1)

	if cv.GetElemType(image) == cv.CV_8UC3:
		cv.CvtColor(image, src, cv.CV_BGR2GRAY)
	else:
		src = image
	
	cv.ConvertScale(src, src_f, 1.0 / 255, 0)

	dest = cv.CloneImage(src_f)
	dest_mag = cv.CloneImage(src_f)

	var = pos_var / 1.0
	w = pos_w / 10.0
	phase = pos_phase * cv.CV_PI / 180.0
	psi = cv.CV_PI * pos_psi / 180.0

	cv.Zero(kernel)
	for x in range(-kernel_size / 2 + 1, kernel_size / 2 + 1):
		for y in range(-kernel_size / 2 + 1, kernel_size / 2 + 1):
			kernel_val = math.exp(-((x * x) + (y * y)) / (2 * var)) * math.cos(w * x * math.cos(phase) + w * y * math.sin(phase) + psi)
			cv.Set2D(kernel, y + kernel_size / 2, x + kernel_size / 2, cv.Scalar(kernel_val))

	cv.Filter2D(src_f, dest, kernel, (-1, -1))

	cv.Pow(dest, dest_mag, 2)

	return (dest_mag, dest)


def data_gen(img_kind, subdir = "data/train/"):
	classes = []
	data = []

	the_ones = glob.glob(subdir + "f_" + img_kind + "*.jpg")
	all_of_them = glob.glob(subdir + "f_*_*.jpg")
	the_others = []

	for x in all_of_them:
		if the_ones.count(x) < 1:
			the_others.append(x)
	
	for x in the_ones:
		classes.append(1)
		data.append(get_image_features(cv.LoadImageM(x)))
	
	for x in the_others:
		classes.append(-1)
		data.append(get_image_features(cv.LoadImageM(x)))

	return (classes, data)

def fit_pca_and_lda(img_kind, data, classes):
	print 'Fiting ' + img_kind

	c_pca = PCA(pca_components)
	c_lda = LDA(lda_components)

	c_pca.fit(data)
	c_lda.fit(data, classes)

	num = len(data)

	for i in xrange(num):

		pca_list = c_pca.transform(data[i]).tolist()[0]
		lda_list = c_lda.transform(data[i]).tolist()[0]

		data_list = [] 
		data_list.extend(pca_list)
		data_list.extend(lda_list)

		data[i] = data_list

	pca[img_kind] = c_pca
	lda[img_kind] = c_lda


def train():

	data_dict = {}

	classes_dict = {}

	global pca
	global lda
	pca = {}
	lda = {}

	print 'Fiting PCA and LDA'
	for img_kind in img_kinds:
		(classes, data) = data_gen(img_kind, train_subdir)

		fit_pca_and_lda(img_kind, data, classes)

		data_dict[img_kind] = data
		classes_dict[img_kind] = classes

	print '================================'

	write_pca_to_file()
	write_lda_to_file()

	global models
	models = {}

	print 'BUILDING TRAIN MODELS'
	for img_kind in img_kinds:
		print "\t" + img_kind
		data = data_dict[img_kind]
		classes = classes_dict[img_kind]
		problem = svm.svm_problem(classes, data)
		param = svm.svm_parameter(svm_params)
		models[img_kind] = svmutil.svm_train(problem, param)
		save_model(models[img_kind], img_kind)
		print_model(img_kind)
	print '================================'

def test():

	total_count = 0
	correct_count = 0
	wrong_count = 0

	print 'TESTING MODELS'

	for img_kind in img_kinds:
		images = glob.glob(test_subdir + "f_" + img_kind + "*.jpg")
		for image in images:
			print "\t" + image
			image_data = get_image_features(cv.LoadImage(image))
			
			(sorted_results, result) = test_image(image_data)

			total_count += 1
			if result == img_kind:
				print 'YES :' + result
				correct_count += 1
			else:
				print 'NO  :' + result
				print sorted_results
				wrong_count += 1
			print '-----------------------'
	print '================================'
	print "Total Pictures: " + str(total_count)
	print "Correct: " + str(correct_count)
	print "Wrong: " + str(wrong_count)
	print "Accuracy: " + str(correct_count/float(total_count) * 100)

def test_image(image_data):
	results = {}

	for kind in img_kinds:

		pca_data = pca[kind].transform([image_data]).tolist()[0]
		lda_data = lda[kind].transform([image_data]).tolist()[0]

		data = []
		data.extend(pca_data)
		data.extend(lda_data)

		predict_input_data = []
		predict_input_data.append(data)

		print 'match: ' + kind

		(val, val_2, label) = svmutil.svm_predict([1] ,predict_input_data, models[kind])
		results[kind] = label[0][0]
			
	sorted_results = sorted(results.iteritems(), key=operator.itemgetter(1))
	result = sorted_results[len(sorted_results)-1][0]

	return sorted_results, result

def get_gray_image(image):
	#image_2 = cv.CreateImage((img_width, img_height), 8, 1)
	#cv.Resize(image, image_2)

	img_gray = cv.CreateImage(cv.GetSize(image), 8, 1)
	cv.CvtColor(image, img_gray, cv.CV_RGB2GRAY)

	cv.EqualizeHist(img_gray, img_gray)

	return img_gray

def build_gray_image(src_dir, dst_dir):
	images = glob.glob(src_dir + "*.jpg")

	for image in images:
		img = cv.LoadImage(image)
		img_gray = None
		try:
			(img_f, img_r) = face.handle_camera_image(img)
			img_gray = img_r
		except TypeError:
			img_gray = get_gray_image(img)
		
		cv.SaveImage(dst_dir + os.path.basename(image), img_gray)

def write_models_to_file():
	for img_kind in img_kinds:
		print "Writing model: " + img_kind
		svmutil.svm_save_model(models_dir + img_kind + '.model', models[img_kind])

def read_models_from_file():
	for img_kind in img_kinds:
		print "Loading model: " + img_kind
		models[img_kind] = svmutil.svm_load_model(models_dir + img_kind + '.model')

def save_model(model, img_kind):
	svmutil.svm_save_model(models_dir + img_kind + '.model', model)

def print_model(img_kind):
	f = open(models_dir + img_kind + '.model')
	for line in f:
		print line
	f.close()

def write_pca_to_file():
	f = open(models_dir + 'pca.pkl', "wb")
	pickle.dump(pca, f)
	f.close()

def read_pca_from_file():
	print 'Loading PCA model'
	f = open(models_dir + 'pca.pkl', "rb")
	global pca
	pca = pickle.load(f)
	f.close()

def write_lda_to_file():
	f = open(models_dir + 'lda.pkl', "wb")
	pickle.dump(lda, f)
	f.close()

def read_lda_from_file():
	print 'Loading LDA model'
	f = open(models_dir + 'lda.pkl', "rb")
	global lda
	lda = pickle.load(f)
	f.close()

def pca_test(img_kind):
	import pylab as pl
	from mpl_toolkits.mplot3d import Axes3D

	(classes, data) = data_gen(img_kind, train_subdir)

	pca = PCA(pca_components, whiten=True)
	print 'fiting'
	pca.fit(data)
	print 'transforming'
	X_r = pca.transform(data)
	print '----'

	print X_r.shape

	x0 = [x[0] for x in X_r]
	x1 = [x[1] for x in X_r]

	pl.figure()

	for i in xrange(0,len(x0)):
		if classes[i] == 1:
			pl.scatter(x0[i], x1[i], c = 'r')
		else:
			pl.scatter(x0[i], x1[i], c = 'b')
	
	pl.legend()
	pl.title('PCA of dataset ' + img_kind)

	pl.show()

def lda_test(img_kind):
	import pylab as pl
	
	(classes, data) = data_gen(img_kind, train_subdir)
	
	lda = LDA(lda_components)
	print 'fiting'
	lda.fit(data, classes)
	print 'transforming'
	X_r = lda.transform(data)
	print '----'

	print X_r.shape

	x0 = [x[0] for x in X_r]
	x1 = [x[1] for x in X_r]

	pl.figure()
	for i in xrange(0,len(x0)):
		if classes[i] == 1:
			pl.scatter(x0[i], x1[i], c = 'r')
		else:
			pl.scatter(x0[i], x1[i], c = 'b')
	

	pl.legend()
	pl.title('LDA of dataset ' + img_kind)

	pl.show()

def gabor_test(file_path):
	import cv2

	image = cv.LoadImage(file_path)

	hcatImage = None

	for phase in gabor_phase:
		vcatImage = None

		for pulsation in gabor_pulsation:

			(t_img_mag, t_img) = gabor(image, kernel_var, pulsation, phase, gabor_psi)
			trans = numpy.asarray(cv.GetMat(t_img_mag))

			if vcatImage == None:
				vcatImage = trans
				continue

			vcatImage = numpy.vstack((vcatImage, trans))

		if hcatImage == None:
			hcatImage = vcatImage
			continue

		hcatImage = numpy.hstack((hcatImage, vcatImage))

	gaborImage = cv2.resize(hcatImage, (int(hcatImage.shape[1] / 1.2), int(hcatImage.shape[0] / 1.2)))

	cv.ShowImage('Image', image)
	cv2.imshow('gabor', gaborImage)
	#cv2.resizeWindow('gabor', hcatImage.shape[1] / 2, hcatImage.shape[0] / 2)

	cv2.waitKey()


def detect_face(img, hc):

	debug = False

	def resize_img(img, x, y, w, h):
		
		new_x = x + abs(w - img_width) / 2
		new_y = y + abs(h - img_height) / 2 + 5

		img_o = cv.GetSubRect(img, (new_x, new_y, img_width, img_height))
		img_r = cv.CreateImage((img_width, img_height), 8, 1)
		cv.Resize(img_o, img_r)
		return img_r
	    

	img2 = cv.CreateMat(cv.GetSize(img)[1] / 2, cv.GetSize(img)[0] / 2, cv.CV_8UC3)
	cv.Resize(img, img2)

	img_gray = cv.CreateImage(cv.GetSize(img2), 8, 1)
	cv.CvtColor(img2, img_gray, cv.CV_RGB2GRAY)
	cv.EqualizeHist(img_gray, img_gray)

	img_f = img_gray

	objects = cv.HaarDetectObjects(img_f, hc, cv.CreateMemStorage(), min_size = (img_width, img_height))
	number_of_faces = len(objects)

	if number_of_faces != 1:
		if debug:
			print "Error! Number of detected faces: " + str(number_of_faces)
		return None
	else:
		for (x, y, w, h), n in objects:

			img_r = resize_img(img_f, x, y, w, h)

			cv.Rectangle(img_f, (x, y), (x + w, y + h), 255)
			if debug:
				print "FACE -> h: " + str(h) + ", w: " + str(w) + ", r(w/h): " + str(float(w) / float(h))

			return (img_f, img_r)

def live_test(capture = None):

	if not capture:
		capture = cv.CaptureFromCAM(0)

	hc = cv.Load(face_cascade)

	i = 0
	while True:
		img = cv.QueryFrame(capture)
		cv.ShowImage("image", img)
		
		returned = detect_face(img, hc)

		if returned == None:
			pass
		else:
			(img_o, img_face) = returned

			if i == 10:
				result = test_image(get_image_features(img_face))[1]
				font=cv.InitFont(cv.CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0, 3, 8)
				cv.PutText(img_o, result, (10, img_o.height - 10), font, (0, 0, 0))
				cv.ShowImage("face",img_o)
				cv.ShowImage("detect", img_face)

		i = i + 1

		if i > 10:
			i = 0

		key_pressed = cv.WaitKey(50)

		if key_pressed == 27:
			break


def test_1():
	src_dir = r'F:/original/'
	dst_dir = r'F:/face/'

	subdirs = os.listdir(src_dir)

	for subdir in subdirs:
		dir = src_dir + subdir + r'/'
		build_gray_image(dir, dst_dir)

def test_2():

	train()
	write_models_to_file()
	write_pca_to_file()
	write_lda_to_file()

def test_3():
	read_models_from_file()
	read_pca_from_file()
	read_lda_from_file()
	test()

def test_4():
	read_models_from_file()
	read_pca_from_file()
	read_lda_from_file()
	live_test()

def test_5():
	img_kind = 'happiness'
	pca_test(img_kind)

def test_5():
	img_kind = 'happiness'
	lda_test(img_kind)

if __name__ == '__main__':
	read_models_from_file()
	read_pca_from_file()
	read_lda_from_file()
	live_test()
	#gabor_test()
	#test_1()
	#test_2()
	#test_3()
	#test_4()
	#test_5()
	#main()
	#models = read_models_from_file()
	#live_test(models)
	pass