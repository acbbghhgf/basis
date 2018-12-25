import os
import sys
import utils
import glob
import cv
import thread
import random
import shutil
from PyQt4 import  QtCore, QtGui, uic

mainwindow_ui = 'ui/mainwindow.ui'


class ConsoleStream(QtCore.QObject):

	textWritten = QtCore.pyqtSignal(str)

	def write(self, text):
		self.textWritten.emit(str(text))

class MainWindow(QtGui.QMainWindow):

	def __init__(self):
		super(MainWindow, self).__init__()
		self.ui = uic.loadUi(mainwindow_ui, self)

		sys.stdout = ConsoleStream()

		sys.stdout.textWritten.connect(self.output)

		self.ui.faceEdit.setText(str(utils.face_cascade))
		self.ui.expressionEdit.setText(str(utils.img_kinds))
		self.ui.kernelEdit.setText(str(utils.kernel_size))
		self.ui.varianceEdit.setText(str(utils.kernel_var))
		self.ui.phaseEdit.setText(str(utils.gabor_psi))
		self.ui.directionEdit.setText(str(utils.gabor_phase))
		self.ui.scaleEdit.setText(str(utils.gabor_pulsation))
		self.ui.pcaEdit.setText(str(utils.pca_components))
		self.ui.ldaEdit.setText(str(utils.lda_components))
		self.ui.svmEdit.setText(str(utils.svm_params))
		self.ui.xEdit.setText(str(utils.img_width))
		self.ui.yEdit.setText(str(utils.img_height))

		utils.read_models_from_file()
		utils.read_pca_from_file()
		utils.read_lda_from_file()

	def __del__(self):
		sys.stdout = sys.__stdout__
		pass

	def output(self, text):
		"""Append text to the QTextEdit."""
		cursor = self.ui.outputEdit.textCursor()
		cursor.movePosition(QtGui.QTextCursor.End)
		cursor.insertText(text)
		self.ui.outputEdit.setTextCursor(cursor)
		self.ui.outputEdit.ensureCursorVisible()

	@QtCore.pyqtSlot()
	def on_faceEdit_editingFinished(self):
		utils.face_cascade = str(self.ui.faceEdit.text())

	@QtCore.pyqtSlot()
	def on_expressionEdit_editingFinished(self):
		l = str(self.ui.expressionEdit.text())
		l = l[2:len(l)-2].split("', '")
		utils.img_kinds = l

	@QtCore.pyqtSlot()
	def on_kernelEdit_editingFinished(self):
		utils.kernel_size = int(self.ui.kernelEdit.text())

	@QtCore.pyqtSlot()
	def on_varianceEdit_editingFinished(self):
		utils.kernel_var = int(self.ui.varianceEdit.text())

	@QtCore.pyqtSlot()
	def on_phaseEdit_editingFinished(self):
		utils.gabor_psi = int(self.ui.phaseEdit.text())

	@QtCore.pyqtSlot()
	def on_directionEdit_editingFinished(self):
		l = str(self.ui.directionEdit.text())
		l = l[1:len(l)-1].split(', ')
		l = [int(x) for x in l]
		utils.gabor_phase = l

	@QtCore.pyqtSlot()
	def on_scaleEdit_editingFinished(self):
		l = str(self.ui.scaleEdit.text())
		l = l[1:len(l)-1].split(', ')
		l = [int(x) for x in l]
		utils.gabor_pulsation = l

	@QtCore.pyqtSlot()
	def on_pcaEdit_editingFinished(self):
		utils.pca_components = int(self.ui.pcaEdit.text())

	@QtCore.pyqtSlot()
	def on_ldaEdit_editingFinished(self):
		utils.lda_components = int(self.ui.ldaEdit.text())

	@QtCore.pyqtSlot()
	def on_svmEdit_editingFinished(self):
		utils.svm_params = str(self.ui.svmEdit.text())

	@QtCore.pyqtSlot()
	def on_xEdit_editingFinished(self):
		utils.img_width = int(self.ui.xEdit.text())

	@QtCore.pyqtSlot()
	def on_yEdit_editingFinished(self):	
		utils.img_height = int(self.ui.yEdit.text())

	@QtCore.pyqtSlot()
	def on_trainAction_triggered(self):
		train_dir, ok = QtGui.QInputDialog.getText(self, 'Train Dir', 'enter train dir: ', QtGui.QLineEdit.Normal, utils.train_subdir)
		if ok and not train_dir.isEmpty():
			utils.train_subdir = str(train_dir)

	@QtCore.pyqtSlot()
	def on_testAction_triggered(self):
		test_dir, ok = QtGui.QInputDialog.getText(self, 'Test Dir', 'enter test dir:', QtGui.QLineEdit.Normal, utils.test_subdir)
		if ok and not test_dir.isEmpty():
			utils.test_subdir = str(test_dir)

	@QtCore.pyqtSlot()
	def on_expressionAction_triggered(self):
		face_dir, ok = QtGui.QInputDialog.getText(self, 'Expressions Dir', 'enter expressions dir:', QtGui.QLineEdit.Normal, utils.face_subdir)
		if ok and not face_dir.isEmpty():
			utils.face_subdir = str(face_dir)

	@QtCore.pyqtSlot()
	def on_generateTrainAction_triggered(self):
		length, ok = QtGui.QInputDialog.getText(self, 'Train Numbers', 'enter train numbers:')
		if ok and not length.isEmpty():
			length = int(length)

			if os.path.exists(utils.train_subdir):
				shutil.rmtree(utils.train_subdir)
			os.mkdir(utils.train_subdir)

			for img_kind in utils.img_kinds:
				images = glob.glob(utils.face_subdir + 'f_' + img_kind + '*.jpg')
				random.shuffle(images)
				images = images[:length]

				for image in images:
					image_gray = utils.get_gray_image(cv.LoadImage(image))
					img = cv.CreateImage((utils.img_width, utils.img_height), 8, 1)
					cv.Resize(image_gray, img)
					image_gray = img
					cv.SaveImage(utils.train_subdir + os.path.basename(image), image_gray)

		QtGui.QMessageBox.information(self, "Train Images", "train images generated.")

		filenames = QtGui.QFileDialog.getOpenFileNames(self, "Train Images", "data/train/", "All files(*.*)")

		if filenames.isEmpty() == False:
			self.on_loadTrainButton_clicked()

	@QtCore.pyqtSlot()
	def on_generateTestAction_triggered(self):
		
		length, ok = QtGui.QInputDialog.getText(self, 'Test Numbers', 'enter test numbers:')
		if ok and not length.isEmpty():
			length = int(length)

			if os.path.exists(utils.test_subdir):
				shutil.rmtree(utils.test_subdir)
			os.mkdir(utils.test_subdir)

			for img_kind in utils.img_kinds:
				images = glob.glob(utils.face_subdir + 'f_' + img_kind + '*.jpg')
				random.shuffle(images)
				images = images[:length]

				for image in images:
					image_gray = utils.get_gray_image(cv.LoadImage(image))
					img = cv.CreateImage((utils.img_width, utils.img_height), 8, 1)
					cv.Resize(image_gray, img)
					image_gray = img
					cv.SaveImage(utils.test_subdir + os.path.basename(image), image_gray)

		QtGui.QMessageBox.information(self, "Test Images", "test images generated.")
		
		filenames = QtGui.QFileDialog.getOpenFileNames(self, "Test Images", "data/test/", "All files(*.*)")

		if filenames.isEmpty() == False:
			self.on_loadTestButton_clicked()


	@QtCore.pyqtSlot()
	def on_aboutAction_triggered(self):
		QtGui.QMessageBox.about(self, 'About', 'Facial expression recognition 1.0 \n Author: Chen Wu')


	@QtCore.pyqtSlot()
	def on_loadTrainButton_clicked(self):
		images = glob.glob(utils.train_subdir + "*.jpg")

		widget = QtGui.QWidget()
		vBoxLayout = QtGui.QVBoxLayout(widget)

		for image in images:

			label = QtGui.QLabel()
			label.setPixmap(QtGui.QPixmap(image))

			textlabel = QtGui.QLabel()
			textlabel.setText(os.path.basename(image))
			textlabel.setAlignment(QtCore.Qt.AlignCenter)

			hBox = QtGui.QHBoxLayout()
			hBox.addWidget(label)
			hBox.addWidget(textlabel)
			vBoxLayout.addLayout(hBox)

		self.ui.scrollArea.setWidget(widget)

	@QtCore.pyqtSlot()
	def on_loadTestButton_clicked(self):
		images = glob.glob(utils.test_subdir + "*.jpg")
		self.textlabels = []
		self.image_data = []
		self.kinds = []

		widget = QtGui.QWidget()
		vBoxLayout = QtGui.QVBoxLayout(widget)

		for image in images:
			self.image_data.append(cv.LoadImage(image))
			self.kinds.append(os.path.splitext(image)[0].split('_')[1])

			label = QtGui.QLabel()
			label.setPixmap(QtGui.QPixmap(image))

			namelabel = QtGui.QLabel()
			namelabel.setText(os.path.basename(image))
			namelabel.setAlignment(QtCore.Qt.AlignCenter)

			textlabel = QtGui.QLabel()
			textlabel.setAlignment(QtCore.Qt.AlignCenter)
			self.textlabels.append(textlabel)

			vBox = QtGui.QVBoxLayout()
			vBox.addWidget(namelabel)
			vBox.addWidget(textlabel)

			w = QtGui.QWidget()
			w.setLayout(vBox)

			hBox = QtGui.QHBoxLayout()
			hBox.addWidget(label)
			hBox.addWidget(w)

			vBoxLayout.addLayout(hBox)

		self.ui.scrollArea.setWidget(widget)

	@QtCore.pyqtSlot()
	def on_trainButton_clicked(self):
		thread.start_new_thread(utils.train, ())
		
	@QtCore.pyqtSlot()
	def on_testButton_clicked(self):
		def test():
			total_count = 0
			correct_count = 0
			wrong_count = 0
			
			images = glob.glob(utils.test_subdir + '*.jpg')

			i = 0
			for data in self.image_data :
				(sorted_results, result) = utils.test_image(utils.get_image_features(data))
				self.textlabels[i].setText(result)

				
				total_count += 1

				if self.kinds[i] == result:
					print 'YES :' + result
					correct_count += 1
				else:
					print 'NO  :' + result
					print sorted_results
					wrong_count += 1

				i += 1

				print '-----------------------'



			print '================================'
			print "Total Pictures: " + str(total_count)
			print "Correct: " + str(correct_count)
			print "Wrong: " + str(wrong_count)
			print "Accuracy: " + str(correct_count / float(total_count) * 100)



		thread.start_new_thread(test, ())

	@QtCore.pyqtSlot()
	def on_videoButton_clicked(self):
		#utils.live_test()
		thread.start_new_thread(utils.live_test, ())

	@QtCore.pyqtSlot()
	def on_gaborTestButton_clicked(self):
		filename = QtGui.QFileDialog.getOpenFileName(self, "Select Gabor Image", "data/train/", "All files(*.*)")
		if filename != False:
			utils.gabor_test(str(filename))


	@QtCore.pyqtSlot()
	def on_pcaTestButton_clicked(self):
		img_kind, ok = QtGui.QInputDialog.getText(self, 'Input Dialog', 'choose one expression')
		if ok and not img_kind.isEmpty():
			img_kind = str(img_kind)
			if img_kind in utils.img_kinds:
				utils.pca_test(img_kind)

	@QtCore.pyqtSlot()
	def on_ldaTestButton_clicked(self):
		img_kind, ok = QtGui.QInputDialog.getText(self, 'Input Dialog', 'choose one expression')
		if ok and not img_kind.isEmpty():
			img_kind = str(img_kind)
			if img_kind in utils.img_kinds:
				utils.lda_test(img_kind)

	@QtCore.pyqtSlot()
	def on_clearButton_clicked(self):
		self.ui.outputEdit.setText('')

if __name__ == '__main__':
	app = QtGui.QApplication( sys.argv )
	mainWindow = MainWindow()
	mainWindow.show()
	app.exec_()