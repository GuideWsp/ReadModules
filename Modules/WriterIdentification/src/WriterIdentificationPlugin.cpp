/*******************************************************************************************************
 BinarizationPlugin.cpp

 nomacs is a fast and small image viewer with the capability of synchronizing multiple instances

 Copyright (C) 2015 #YOUR_NAME

 This file is part of nomacs.

 nomacs is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 nomacs is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *******************************************************************************************************/

#include "WriterIdentificationPlugin.h"

 // nomacs includes
#include "DkSettings.h"
#include "DkImageStorage.h"

#include "WriterIdentification.h"
#include "WIDatabase.h"
#include "Image.h"
#include "Settings.h"


#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#include <QSettings>
#include "opencv2/features2d/features2d.hpp"
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
WriterIdentificationPlugin::WriterIdentificationPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[id_calcuate_features] = "1bb00c5713a849eebb9ea16fcf794740";
	runIds[id_generate_vocabulary] = "aa8cf182dc4348aa9917cd3c3fc95d8c";
	runIds[id_identify_writer] = "b9fc66129483473fa901ddf627bd8b9a";
	runIds[id_evaluate_database] = "e247a635ebb3449ba88204abf8d5f089";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_calcuate_features] = tr("Calcuate Features");
	menuNames[id_generate_vocabulary] = tr("Generate Vocabulary");
	menuNames[id_identify_writer] = tr("Identify Writer");
	menuNames[id_evaluate_database] = tr("Evaluate Database");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_calcuate_features] = tr("Calculates the features for writer identification on this page");
	statusTips[id_generate_vocabulary] = tr("Generates a new vocabulary using the given pages");
	statusTips[id_identify_writer] = tr("Identifies the writer of the given page");
	statusTips[id_evaluate_database] = tr("Evaluates the selected files");
	mMenuStatusTips = statusTips.toList();

	init();
}
/**
*	Destructor
**/
WriterIdentificationPlugin::~WriterIdentificationPlugin() {

	qDebug() << "destroying WriterIdentification plugin...";
}


/**
* Returns unique ID for the generated dll
**/
QString WriterIdentificationPlugin::id() const {

	return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage WriterIdentificationPlugin::image() const {

	return QImage(":/WriterIdentificationPlugin/img/read.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString WriterIdentificationPlugin::version() const {

	return PLUGIN_VERSION;
};

QList<QAction*> WriterIdentificationPlugin::createActions(QWidget* parent) {

	if (mActions.empty()) {

		for (int idx = 0; idx < id_end; idx++) {
			QAction* ca = new QAction(mMenuNames[idx], parent);
			ca->setObjectName(mMenuNames[idx]);
			ca->setStatusTip(mMenuStatusTips[idx]);
			ca->setData(mRunIDs[idx]);	// runID needed for calling function runPlugin()
			mActions.append(ca);
		}
	}

	return mActions;
}



QList<QAction*> WriterIdentificationPlugin::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> WriterIdentificationPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC, QSharedPointer<nmc::DkBatchInfo>& info) const {

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[id_calcuate_features]) {
		qInfo() << "calculating features for writer identification";
		WriterIdentification wi = WriterIdentification();
		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		wi.setImage(imgCv);
		wi.calculateFeatures();
		cv::cvtColor(imgCv, imgCv, CV_RGB2GRAY);
		QVector<cv::KeyPoint> kp = wi.getKeyPoints();
		for(int i = 0; i < kp.length(); i++) {
			kp[i].size *= 1.5 * 4;
		}
		cv::drawKeypoints(imgCv, kp.toStdVector(), imgCv, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
		//cv::drawKeypoints(imgCv, wi.getKeyPoints().toStdVector(), imgCv, cv::Scalar::all(-1));
		
		QString fFilePath = featureFilePath(imgC->filePath(), true);
		wi.saveFeatures(fFilePath);

		//QImage img = nmc::DkImage::mat2QImage(imgCv);
		//img = img.convertToFormat(QImage::Format_ARGB32);
		//imgC->setImage(img, tr("SIFT keypoints"));
	}
	else if(runID == mRunIDs[id_generate_vocabulary]) {
		qInfo() << "collecting files for vocabulary generation";

		QString ffPath = featureFilePath(imgC->filePath());
		ffPath.replace(ffPath.length() - 4, ffPath.length(), ".yml");

		int idxOfMinus = QFileInfo(imgC->filePath()).baseName().indexOf("-");
		int idxOfUScore = QFileInfo(imgC->filePath()).baseName().indexOf("_");
		int idx = -1;
		if(idxOfMinus == -1 && idxOfUScore > 0)
			idx = idxOfUScore;
		else if(idxOfUScore == -1 && idxOfMinus > 0)
			idx = idxOfMinus;
		else if(idxOfMinus > 0 && idxOfUScore > 0)
			idx = idxOfMinus > idxOfUScore ? idxOfMinus : idxOfUScore;
		QString label = QFileInfo(imgC->filePath()).baseName().left(idx);

		QSharedPointer<WIInfo> wInfo(new WIInfo(runID, imgC->filePath()));
		wInfo->setWriter(label);
		wInfo->setFeatureFilePath(ffPath);

		info = wInfo;
	}
	else if(runID == mRunIDs[id_identify_writer]) {
		qInfo() << "identifying writer";
	}
	else if(runID == mRunIDs[id_evaluate_database]) {
		qInfo() << "collecting files evaluation";

		QString fFilePath = featureFilePath(imgC->filePath());

		if(QFileInfo(fFilePath).exists()) {

			int idxOfMinus = QFileInfo(imgC->filePath()).baseName().indexOf("-");
			int idxOfUScore = QFileInfo(imgC->filePath()).baseName().indexOf("_");
			int idx = -1;
			if(idxOfMinus == -1 && idxOfUScore > 0)
				idx = idxOfUScore;
			else if(idxOfUScore == -1 && idxOfMinus > 0)
				idx = idxOfMinus;
			else if(idxOfMinus > 0 && idxOfUScore > 0)
				idx = idxOfMinus > idxOfUScore ? idxOfMinus : idxOfUScore;
			QString label = QFileInfo(imgC->filePath()).baseName().left(idx);

			QSharedPointer<WIInfo> wInfo(new WIInfo(runID, imgC->filePath()));
			wInfo->setWriter(label);
			wInfo->setFeatureFilePath(fFilePath);

			info = wInfo;
		} else {
			qDebug() << "no features files exists for image: " << imgC->filePath()  << "... skipping";
		}
	}


	// wrong runID? - do nothing
	return imgC;
}
void WriterIdentificationPlugin::preLoadPlugin() const {
	qDebug() << "preloading plugin";
	
}
void WriterIdentificationPlugin::postLoadPlugin(const QVector<QSharedPointer<nmc::DkBatchInfo> >& batchInfo) const {
	qDebug() << "postLoadPlugin";

	if(batchInfo.empty())
		return;

	int runIdx = mRunIDs.indexOf(batchInfo.first()->id());

	if(runIdx == -1) {
		qWarning() << "unknown run id: " << batchInfo.first()->id();
		return;
	}

	qDebug() << "[POST LOADING]" << mMenuNames[runIdx] << "--------------------------------------------";

	if(runIdx == id_generate_vocabulary) {
		WIDatabase wiDatabase = WIDatabase();
		WIVocabulary voc = WIVocabulary();
		if(mVocType != WIVocabulary::WI_UNDEFINED) {
			voc.setType(mVocType);
			voc.setNumberOfCluster(mVocNumberOfClusters);
			voc.setNumberOfPCA(mVocNumberOfPCA);
		}
		else {
			qDebug() << "vocabulary in settings file undefined. Using default values";
			voc.setType(WIVocabulary::WI_GMM);
			voc.setNumberOfCluster(40);
			voc.setNumberOfPCA(64);

			//voc.setType(WIVocabulary::WI_BOW);
			//voc.setNumberOfCluster(300);
			//voc.setNumberOfPCA(0);
		}

		wiDatabase.setVocabulary(voc);

		QStringList classLabels, featurePaths;
		for(auto bi : batchInfo) {
			WIInfo * wInfo = dynamic_cast<WIInfo*>(bi.data());
			wiDatabase.addFile(wInfo->featureFilePath());
			featurePaths.append(wInfo->featureFilePath());
			classLabels.append(wInfo->writer());
		}

		wiDatabase.generateVocabulary();
		wiDatabase.saveVocabulary(mVocType == WIVocabulary::WI_UNDEFINED ? "C://tmp//voc-woSettings.yml" : mSettingsVocPath);
		wiDatabase.evaluateDatabase(classLabels, featurePaths);
	}
	else if(runIdx == id_evaluate_database) {
		WIDatabase wiDatabase = WIDatabase(); 
		WIVocabulary voc = WIVocabulary();
		if(mVocType == WIVocabulary::WI_UNDEFINED)
			qDebug() << "vocabulary path not set in settings file. Using default path";
		voc.loadVocabulary(mVocType == WIVocabulary::WI_UNDEFINED ? "C://tmp//voc-woSettings.yml" : mSettingsVocPath);
		wiDatabase.setVocabulary(voc);
		QStringList classLabels, featurePaths;
		for(auto bi : batchInfo) {
			WIInfo * wInfo = dynamic_cast<WIInfo*>(bi.data());
			wiDatabase.addFile(wInfo->featureFilePath());
			featurePaths.append(wInfo->featureFilePath());
			classLabels.append(wInfo->writer());
		}
		wiDatabase.evaluateDatabase(classLabels, featurePaths);
	}
}

void WriterIdentificationPlugin::init() {
	loadSettings(nmc::Settings::instance().getSettings());
}

void WriterIdentificationPlugin::loadSettings(QSettings & settings) {
	settings.beginGroup("WriterIdentification");
	mSettingsVocPath = settings.value("vocPath", QString()).toString();
	mVocType = settings.value("vocType", WIVocabulary::WI_UNDEFINED).toInt();
	if(mVocType > WIVocabulary::WI_UNDEFINED)
		mVocType = WIVocabulary::WI_UNDEFINED;
	mVocNumberOfClusters = settings.value("numberOfClusters", -1).toInt();
	mVocNumberOfPCA = settings.value("numberOfPCA", -1).toInt();
	mFeatureDir = settings.value("featureDir", QString()).toString();
	qDebug() << "settings read: path: " << mSettingsVocPath << " type:" << mVocType << " numberOfClusters:" << mVocNumberOfClusters << " numberOfPCA: " << mVocNumberOfPCA;
	settings.endGroup();
}

void WriterIdentificationPlugin::saveSettings(QSettings & settings) const {
	settings.beginGroup("WriterIdentification");

	settings.endGroup();
}

QString WriterIdentificationPlugin::featureFilePath(QString imgPath, bool createDir) const {
	if(mFeatureDir.isEmpty()) {
		QString featureFilePath = imgPath;
		return featureFilePath.replace(featureFilePath.length() - 4, featureFilePath.length(), ".yml");
	}
	else {
		QString ffPath;
		QFileInfo fImgPath(imgPath);
		QFileInfo fFeatPath(mFeatureDir);
		if(fFeatPath.isAbsolute()) {
			ffPath = fFeatPath.absoluteDir().path() + fImgPath.baseName();
		}
		else {
			QFileInfo combined(fImgPath.absoluteDir().path() + "/" + mFeatureDir + "/");
			if(!combined.exists() && createDir) {
				QDir dir(fImgPath.absoluteDir());
				if(!dir.mkdir(mFeatureDir)) {
					qDebug() << "unable to create subdirectory";
				}
			}
			ffPath = combined.absoluteDir().path() + "/" + fImgPath.baseName() + ".yml";
		}
		qDebug() << "fFPath.isAbsolute():" << fFeatPath.isAbsolute() << " fFPath.isRelative():" << fFeatPath.isRelative();

		return ffPath;
	}
}

// WIInfo --------------------------------------------------------------------
WIInfo::WIInfo(const QString& id, const QString & filePath) : nmc::DkBatchInfo(id, filePath) {
}

void WIInfo::setWriter(const QString & p) {
	mWriter = p;
}

QString WIInfo::writer() const {
	return mWriter;
}

void WIInfo::setFeatureFilePath(const QString & p) {
	mFeatureFilePath = p;
}

QString WIInfo::featureFilePath() const {
	return mFeatureFilePath;
}

};




