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

#include "BinarizationPlugin.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
BinarizationPlugin::BinarizationPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[id_binarize_otsu] = "4398d8e26fe9454384432e690b47d4d3";
	runIds[id_binarize_su] = "73c1efff27c043d298d8acd99530af1d";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_binarize_otsu] = tr("&Otsu Threshold");
	menuNames[id_binarize_su] = tr("&Su Binarization");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_binarize_otsu] = tr("Thresholds a document with the famous Otsu method");
	statusTips[id_binarize_su] = tr("Thresholds a document with the Su method");
	mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
BinarizationPlugin::~BinarizationPlugin() {
}


/**
* Returns unique ID for the generated dll
**/
QString BinarizationPlugin::pluginID() const {

	return PLUGIN_ID;
};


/**
* Returns plugin name
* @param plugin ID
**/
QString BinarizationPlugin::pluginName() const {

	return tr("&Document Binarization");
};

/**
* Returns long description for every ID
* @param plugin ID
**/
QString BinarizationPlugin::pluginDescription() const {

	return "<b>Created by:</b> #YOUR_NAME <br><b>Modified:</b>04.02.2016<br><b>Description:</b> This Plugin thresholds images with an extended Su et al. binarization..";
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage BinarizationPlugin::pluginDescriptionImage() const {

	return QImage(":/#PLUGIN_NAME/img/your-image.png");
};

/**
* Returns plugin version for every ID
* @param plugin ID
**/
QString BinarizationPlugin::pluginVersion() const {

	return PLUGIN_VERSION;
};

/**
* Returns unique IDs for every plugin in this dll
**/
QStringList BinarizationPlugin::runID() const {

	//GUID without hyphens generated at http://www.guidgenerator.com/
	return QStringList() << PLUGIN_ID;
};

/**
* Returns plugin name for every ID
* @param plugin ID
**/
QString BinarizationPlugin::pluginMenuName(const QString &runID) const {

	return tr("Document Binarization");
};

/**
* Returns short description for status tip for every ID
* @param plugin ID
**/
QString BinarizationPlugin::pluginStatusTip(const QString &runID) const {

	return tr("READ binarization module.");
};

QList<QAction*> BinarizationPlugin::createActions(QWidget* parent) {

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



QList<QAction*> BinarizationPlugin::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> BinarizationPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const {

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[id_binarize_otsu]) {
		imgC->setImage(imgC->image().mirrored(), tr("Otsu Binarization"));
	}
	//else if(runID == mRunIDs[id_binarize_su]) {
	//	imgC->setImage(imgC->image().mirrored(true), tr("Su Binarization"));
	//}

	// wrong runID? - do nothing
	return imgC;
};

};

