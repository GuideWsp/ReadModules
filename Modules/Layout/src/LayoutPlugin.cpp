/*******************************************************************************************************
ReadModules are plugins for nomacs developed at CVL/TU Wien for the EU project READ. 

Copyright (C) 2016 Markus Diem <diem@caa.tuwien.ac.at>
Copyright (C) 2016 Stefan Fiel <fiel@caa.tuwien.ac.at>
Copyright (C) 2016 Florian Kleber <kleber@caa.tuwien.ac.at>

This file is part of ReadModules.

ReadFramework is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ReadFramework is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

The READ project  has  received  funding  from  the European  Union�s  Horizon  2020  
research  and innovation programme under grant agreement No 674943

related links:
[1] http://www.caa.tuwien.ac.at/cvl/
[2] https://transkribus.eu/Transkribus/
[3] https://github.com/TUWien/
[4] http://nomacs.org
*******************************************************************************************************/

#include "LayoutPlugin.h"

// ReadFramework
#include "SuperPixel.h"
#include "LineTrace.h"
#include "Algorithms.h"
#include "Binarization.h"
#include "SkewEstimation.h"

// nomacs
#include "DkImageStorage.h"

#pragma warning(push, 0)	// no warnings from includes - begin
#include <QAction>
#pragma warning(pop)		// no warnings from includes - end

namespace rdm {

/**
*	Constructor
**/
LayoutPlugin::LayoutPlugin(QObject* parent) : QObject(parent) {

	// create run IDs
	QVector<QString> runIds;
	runIds.resize(id_end);

	runIds[id_layout_draw] = "4cbb58ada14d4b64a17fe3285696446b";
	runIds[id_layout_xml] = "b56790b60a904a32975621e4b54ab939";
	runIds[id_lines] = "9af887d7003c44e999ba2db50d65ec85";
	mRunIDs = runIds.toList();

	// create menu actions
	QVector<QString> menuNames;
	menuNames.resize(id_end);

	menuNames[id_layout_draw] = tr("Draw Layout");
	menuNames[id_layout_xml] = tr("PAGE Xml");
	menuNames[id_lines] = tr("Calculate Lines");
	mMenuNames = menuNames.toList();

	// create menu status tips
	QVector<QString> statusTips;
	statusTips.resize(id_end);

	statusTips[id_layout_draw] = tr("Draws the current layout outputs to the image");
	statusTips[id_layout_xml] = tr("Writes the layout analysis results to an XML");
	statusTips[id_lines] = tr("Calculates the lines in the binarized image");
	mMenuStatusTips = statusTips.toList();
}
/**
*	Destructor
**/
LayoutPlugin::~LayoutPlugin() {

	qDebug() << "destroying binarization plugin...";
}


/**
* Returns unique ID for the generated dll
**/
QString LayoutPlugin::id() const {

	return PLUGIN_ID;
};

/**
* Returns descriptive iamge for every ID
* @param plugin ID
**/
QImage LayoutPlugin::image() const {

	return QImage(":/LayoutPlugin/img/read.png");
};

QList<QAction*> LayoutPlugin::createActions(QWidget* parent) {

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



QList<QAction*> LayoutPlugin::pluginActions() const {
	return mActions;
}

/**
* Main function: runs plugin based on its ID
* @param plugin ID
* @param image to be processed
**/
QSharedPointer<nmc::DkImageContainer> LayoutPlugin::runPlugin(const QString &runID, QSharedPointer<nmc::DkImageContainer> imgC) const {

	if (!imgC)
		return imgC;

	if(runID == mRunIDs[id_layout_draw]) {

		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());
		
		imgCv = compute(imgCv);

		QImage img = nmc::DkImage::mat2QImage(imgCv);
		imgC->setImage(img, tr("Layout Analysis Visualized"));
	}
	else if(runID == mRunIDs[id_layout_xml]) {


		qDebug() << "not implemented yet - sorry";
	}
	else if (runID == mRunIDs[id_lines]) {

		cv::Mat imgCv = nmc::DkImage::qImage2Mat(imgC->image());

		if (imgCv.depth() != CV_8U) {
			imgCv.convertTo(imgCv, CV_8U, 255);
			
		}

		if (imgCv.channels() != 1) {
			cv::cvtColor(imgCv, imgCv, CV_RGB2GRAY);
		}
		
		//if mask will is estimated
		//cv::Mat mask = rdf::Algorithms::instance().estimateMask(imgCv);
		cv::Mat mask = cv::Mat();

		//if skew will be used
		//rdf::BaseSkewEstimation bse;
		//bse.setImages(imgCv);
		//bse.setFixedThr(false);
		//if (!bse.compute()) {
		//	qWarning() << "could not compute skew";
		//}
		//double skewAngle = bse.getAngle();
		//skewAngle = skewAngle / 180.0 * CV_PI; //check if minus angle is needed....
		double skewAngle = 0.0f;
		
		rdf::BinarizationSuAdapted binarizeImg(imgCv, mask);
		binarizeImg.compute();
		cv::Mat bwImg = binarizeImg.binaryImage();

		rdf::LineTrace lt(bwImg, mask);
		lt.setAngle(skewAngle);

		lt.compute();

		//cv::Mat lImg = lt.lineImage();
		cv::Mat synLine = lt.generatedLineImage();
		QVector<rdf::Line> hlines = lt.getHLines();
		QVector<rdf::Line> vlines = lt.getVLines();

		//TODO
		//save lines to xml

		//visualize
		QImage img = nmc::DkImage::mat2QImage(synLine);
		imgC->setImage(img, tr("Calculated lines"));

	}

	// wrong runID? - do nothing
	return imgC;
}

cv::Mat LayoutPlugin::compute(const cv::Mat & src) const {
	
	//// TODO: add layout here...

	//return src;

	rdf::SuperPixel sp(src);
	sp.compute();

	qDebug() << "seam carving applied";

	return sp.binaryImage();

}

};

