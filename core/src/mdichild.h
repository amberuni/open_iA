/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2019  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
*                          Amirkhanov, J. Weissenböck, B. Fröhler, M. Schiwarth       *
* *********************************************************************************** *
* This program is free software: you can redistribute it and/or modify it under the   *
* terms of the GNU General Public License as published by the Free Software           *
* Foundation, either version 3 of the License, or (at your option) any later version. *
*                                                                                     *
* This program is distributed in the hope that it will be useful, but WITHOUT ANY     *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A     *
* PARTICULAR PURPOSE.  See the GNU General Public License for more details.           *
*                                                                                     *
* You should have received a copy of the GNU General Public License along with this   *
* program.  If not, see http://www.gnu.org/licenses/                                  *
* *********************************************************************************** *
* Contact: FH OÖ Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstraße 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
#pragma once

#include "defines.h"
#include "qthelper/iAQTtoUIConnector.h"
#include "iAPreferences.h"
#include "iARenderSettings.h"
#include "iASlicerSettings.h"
#include "iAVolumeSettings.h"
#include "open_iA_Core_export.h"
#include "ui_logs.h"
#include "ui_Mdichild.h"
#include "ui_renderer.h"

#include <vtkSmartPointer.h>

#include <QFileInfo>
#include <QMainWindow>
#include <QMap>
#include <QString>
#include <QSharedPointer>

#include <vector>

class QProgressBar;

class vtkAbstractTransform;
class vtkActor;
class vtkColorTransferFunction;
class vtkCornerAnnotation;
class vtkImageCast;
class vtkImageData;
class vtkPiecewiseFunction;
class vtkPoints;
class vtkPolyData;
class vtkScalarsToColors;
class vtkTransform;

class dlg_function;
class dlg_imageproperty;
class dlg_modalities;
class dlg_periodicTable;
class dlg_profile;
class dlg_slicer;
class dlg_volumePlayer;
class iAAlgorithm;
class iAChannelData;
class iADiagramFctWidget;
class iADockWidgetWrapper;
class iAIO;
class iALogger;
class iAModality;
class iAModalityList;
class iAParametricSpline;
class iAPlot;
struct iAProfileProbe;
class iARenderer;
class iASlicer;
class iAVolumeStack;
class MainWindow;

typedef iAQTtoUIConnector<QDockWidget, Ui_renderer>  dlg_renderer;
typedef iAQTtoUIConnector<QDockWidget, Ui_logs>   dlg_logs;

class open_iA_Core_API MdiChild : public QMainWindow, public Ui_Mdichild
{
	Q_OBJECT
public:
	QProgressBar * pbar;

	MdiChild(MainWindow * mainWnd, iAPreferences const & preferences, bool unsavedChanges);
	~MdiChild();

	void showPoly();
	bool loadFile(const QString &f, bool isStack);
	bool loadRaw(const QString &f);
	bool displayResult(QString const & title, vtkImageData* image = nullptr, vtkPolyData* poly = nullptr);
	void prepareForResult();
	bool save();
	bool saveAs();
	bool saveFile(const QString &f, int modalityNr, int componentNr);
	void updateLayout();

	//! waits for the IO thread to finish in case any I/O operation is running; otherwise it will immediately exit
	void waitForPreviousIO();

	void multiview() { changeVisibility(MULTI); }
	bool editPrefs(iAPreferences const & p);
	void applyViewerPreferences();
	bool editRendererSettings(iARenderSettings const & rs, iAVolumeSettings const & vs);
	bool editSlicerSettings(iASlicerSettings const & slicerSettings);
	bool loadTransferFunction();
	bool saveTransferFunction();

	//! Provides the possibility to save a raycaster movie of the given raycaster view.
	//! @param [in] raycaster the VTK raycaster the movie shall be exported from.
	void saveMovie(iARenderer& raycaster);
	int  deletePoint();
	void changeColor();
	void resetView();
	void resetTrf();
	void toggleSnakeSlicer(bool isEnabled);
	bool isSnakeSlicerToggled() const;
	void toggleSliceProfile(bool isEnabled);
	bool isSliceProfileToggled(void) const;
	void enableInteraction(bool b);
	void setRenderSettings(iARenderSettings const & rs, iAVolumeSettings const & vs);
	void setupSlicers(iASlicerSettings const & ss, bool init);
	void check2DMode();
	iALogger * getLogger();
	iARenderSettings const & getRenderSettings() const;
	iAVolumeSettings const & getVolumeSettings() const;
	iASlicerSettings const & getSlicerSettings() const;
	iAPreferences    const & getPreferences()    const;
	iAVolumeStack * getVolumeStack();
	void connectThreadSignalsToChildSlots(iAAlgorithm* thread);
	void connectIOThreadSignals(iAIO* thread);
	void connectAlgorithmSignalsToChildSlots(iAAlgorithm* thread);

	//! Access the opacity function of the "main image"
	//! @deprecated all access to images should proceed via modalities (getModality / setModalities /...)
	vtkPiecewiseFunction * getPiecewiseFunction();
	//! Access the color transfer function of the "main image"
	//! @deprecated all access to images should proceed via modalities (getModality / setModalities /...)
	vtkColorTransferFunction * getColorTransferFunction();
	//! Access to the "main image"
	//! @deprecated retrieve images via the modalities (getModality etc.) instead!
	vtkImageData* getImageData();
	//! Access to the "main image"
	//! @deprecated retrieve images via the modalities (getModality etc.) instead!
	vtkSmartPointer<vtkImageData> getImagePointer();
	//! Set "main image" - does not update views (see displayResult for a method that does)!
	//! @deprecated all access to images should proceed via modalities (getModality / setModalities /...) or channels (addChannel/updateChannel)
	void setImageData(vtkImageData * iData);
	//! @deprecated all access to images should proceed via modalities (getModality / setModalities /...) or channels (addChannel/updateChannel)
	void setImageData(QString const & filename, vtkSmartPointer<vtkImageData> imgData);
	//! Access to "main" polydata object (if any)
	// TODO: move out of mdi child, into something like an iAModality
	vtkPolyData* getPolyData();
	//! Access to the 3D renderer widget
	iARenderer* getRenderer();
	//! Access slicer for given mode (use iASlicerMode enum for mode values)
	iASlicer* slicer(int mode);
	//! Get current slice number in the respective slicer
	int sliceNumber(int mode) const;
	//! Access slicer dock widget for the given mode
	//! @param mode slicer to access - use constants from iASlicerMode enum
	dlg_slicer * slicerDlg(int mode);
	//! Access 3D renderer dock widget
	dlg_renderer * getRendererDlg();
	//! Access image property dock widget
	dlg_imageproperty * getImagePropertyDlg();
	//! Access line profile dock widget
	dlg_profile * getProfileDlg();
	//! Access log message dock widget
	dlg_logs * getLogDlg();

	void setReInitializeRenderWindows(bool reInit) { reInitializeRenderWindows = reInit; }
	vtkTransform* getSlicerTransform();
	bool getResultInNewWindow() const { return preferences.ResultInNewWindow; }
	bool getLinkedMDIs() const { return slicerSettings.LinkMDIs; }
	bool getLinkedViews() const { return slicerSettings.LinkViews; }
	std::vector<dlg_function*> &getFunctions();
	void redrawHistogram();
	iADiagramFctWidget* getHistogram();
	iADockWidgetWrapper* getHistogramDockWidget();

	int getSelectedFuncPoint();
	int isFuncEndPoint(int index);
	void setHistogramFocus();
	bool isMaximized();

	void updateROI(int const roi[6]);
	void setROIVisible(bool visible);

	void setCurrentFile(const QString &f);
	QString userFriendlyCurrentFile();
	QString currentFile() const { return curFile; }
	QFileInfo getFileInfo() const { return fileInfo; }
	QString getFilePath() const;

	//! @{ Multi-Channel rendering
	//! Create a new channel, return its ID.
	uint createChannel();
	//! Update the data of the given channel ID.
	void updateChannel(uint id, vtkSmartPointer<vtkImageData> imgData, vtkScalarsToColors* ctf, vtkPiecewiseFunction* otf);
	//! Update opacity of the given channel ID.
	void updateChannelOpacity(uint id, double opacity);
	
	void setChannelRenderingEnabled(uint, bool enabled);
	iAChannelData * getChannelData(uint id);
	iAChannelData const * getChannelData(uint id) const;
	void initChannelRenderer(uint id, bool use3D, bool enableChannel = true);
	void updateChannelMappers();
	//! @}

	//! @{ slicer pie glyphs - move to XRF module!
	void setSlicerPieGlyphsEnabled(bool isOn);
	void setPieGlyphParameters(double opacity, double spacing, double magFactor);
	//! @}

	//! @{ Magic Lens
	void toggleMagicLens(bool isEnabled);
	bool isMagicLensToggled(void) const;
	void setMagicLensInput(uint id, bool initReslicer);
	void setMagicLensEnabled(bool isOn);
	void reInitMagicLens(uint id, vtkSmartPointer<vtkImageData> imgData, vtkScalarsToColors* ctf, vtkPiecewiseFunction* otf);
	int  getMagicLensSize() const { return preferences.MagicLensSize; }
	int  getMagicLensFrameWidth() const { return preferences.MagicLensFrameWidth; }
	//! @}

	//! Returns the currently active renderer (Default / Raycaster / GPU).
	int getRenderMode() const;

	//! Current coordinate position (defined by the respective slice number in the slice views).
	int const * position() const;

	MainWindow* getMainWnd();
	//! Hides the histogram dockwidget
	void hideHistogram();
	//! Apply current rendering settings to the given iARenderer.
	void applyRenderSettings(iARenderer* raycaster);
	//! Apply current volume settings to all modalities in the current list in dlg_modalities.
	void applyVolumeSettings(const bool loadSavedVolumeSettings);
	//! Returns the name of the layout currently applied to this child window.
	QString getLayoutName() const;
	//! Loads the layout with the given name from the settings store, and tries to restore the according dockwidgets configuration
	void loadLayout(QString const & layout);

	//! If more than one modality loaded, ask user to choose one of them.
	//! (currently used for determining which modality to save)
	int chooseModalityNr(QString const & caption = "Choose Channel");
	//! If given modality has more than one component, ask user to choose one of them.
	//! (currently used for determining which modality to save)
	int chooseComponentNr(int modalityNr);

	//! workaround for bug in splitDockWidget (see https://bugreports.qt.io/browse/QTBUG-60093)
	//! splitDockWidget would makes ref and newWidget disappear if ref is tabbed at the moment
	//void splitDockWidget(QDockWidget* ref, QDockWidget* newWidget, Qt::Orientation orientation);

	//! Checks whether the main image data is fully loaded.
	bool isFullyLoaded() const;

	//! Save all currently loaded files into a project with the given file name.
	void saveProject(QString const & fileName);

	//! Whether volume data is loaded (only checks filename and volume dimensions).
	bool isVolumeDataLoaded() const;

	//! Enable or disable linked slicers and 3D renderer.
	void linkViews(bool l);
	
	//! Enable or disable linked MDI windows for this MDI child.
	void linkMDIs(bool lm);

	//! clear current histogram (i.e. don't show it anymore)
	void clearHistogram();

	void setModalities(QSharedPointer<iAModalityList> modList);
	QSharedPointer<iAModalityList> getModalities();
	QSharedPointer<iAModality> getModality(int idx);
	dlg_modalities* getModalitiesDlg();
	void storeProject();

Q_SIGNALS:
	void rendererDeactivated(int c);
	void pointSelected();
	void noPointSelected();
	void endPointSelected();
	void active();
	void magicLensToggled( bool isToggled );
	void closed();
	void viewsUpdated();
	void renderSettingsChanged();
	void preferencesChanged();
	void viewInitialized();
	void transferFunctionChanged();
	void fileLoaded();
	void histogramAvailable();
public slots:
	void maximizeRC();
	void maximizeXY();
	void maximizeXZ();
	void maximizeYZ();
	void updateProgressBar(int i);
	void hideProgressBar();
	void initProgressBar();
	void disableRenderWindows(int ch);
	void enableRenderWindows();
	void updateSlicer(int index);
	void updateSlicers();
	void updateViews();
	void addMsg(QString txt);
	void addStatusMsg(QString txt);
	void setupView(bool active = false);
	void setupStackView(bool active = false);
	void setupProject(bool active = false);
	bool updateVolumePlayerView(int updateIndex, bool isApplyForAll);
	void removeFinishedAlgorithms();
	void camPX();
	void camPY();
	void camPZ();
	void camMX();
	void camMY();
	void camMZ();
	void camIso();

	//! Calls the getCamPosition function of iARenderer (described there in more detail).
	//!
	//! \param camOptions	All informations of the camera stored in a double array
	void getCamPosition(double * camOptions);

	//! Calls the setCamPosition function of iARenderer (described there in more detail).
	//!
	//! \param camOptions	All informations of the camera stored in a double array
	//! \param rsParallelProjection	boolean variable to determine if parallel projection option on.
	void setCamPosition(double * camOptions, bool rsParallelProjection);
	void updateProbe(int ptIndex, double * newPos);
	void resetLayout();

private slots:
	void saveRC();
	void saveMovRC();
	void triggerInteractionRaycaster();
	void setSliceXY(int s);
	void setSliceYZ(int s);
	void setSliceXZ(int s);
	void setSliceXYScrollBar(int s);
	void setSliceYZScrollBar(int s);
	void setSliceXZScrollBar(int s);
	void setSliceXYSpinBox(int s);
	void setSliceYZSpinBox(int s);
	void setSliceXZSpinBox(int s);
	void setChannel(int ch);
	void setRotationXY(double a);
	void setRotationYZ(double a);
	void setRotationXZ(double a);
	void setSlabModeXY(bool slabMode);
	void setSlabModeYZ(bool slabMode);
	void setSlabModeXZ(bool slabMode);
	void updateSlabThicknessXY(int thickness);
	void updateSlabThicknessYZ(int thickness);
	void updateSlabThicknessXZ(int thickness);
	void updateSlabCompositeModeXY(int compositeMode);
	void updateSlabCompositeModeXZ(int compositeMode);
	void updateSlabCompositeModeYZ(int compositeMode);
	void updateRenderWindows(int channels);
	void updatePositionMarker(int x, int y, int z, int mode);
	void toggleArbitraryProfile(bool isChecked);
	void ioFinished();
	void updateImageProperties();
	void clearLogs();
	void modalityTFChanged();
	void histogramDataAvailable(int modalityIdx);
	void statisticsAvailable(int modalityIdx);
	void changeMagicLensModality(int chg);
	void changeMagicLensOpacity(int chg);
	void changeMagicLensSize(int chg);
	void showModality(int modIdx);
	void saveFinished();
	void modalityAdded(int modalityIdx);

private:
	void closeEvent(QCloseEvent *event);
	void addImageProperty( );
	bool addVolumePlayer(iAVolumeStack *volumeStack);
	void addProfile();
	void updateProfile();
	bool saveAs(int modalityNr);
	bool initView(QString const & title);
	int  evaluatePosition(int pos, int i, bool invert = false);

	//! Changes the display of views from full to multi screen or multi screen to fullscreen.
	//! @param mode how the views should be arranged.
	void changeVisibility(unsigned char mode);
	int  getVisibility() const;
	void hideVolumeWidgets();
	void setVisibility(QList<QWidget*> widgets, bool show);
	void cleanWorkingAlgorithms();
	void maximizeDockWidget(QDockWidget * dw);
	void demaximizeDockWidget(QDockWidget * dw);
	void resizeDockWidget(QDockWidget * dw);

	void connectSignalsToSlots();
	void setRenderWindows();
	void updateSnakeSlicer(QSpinBox* spinBox, iASlicer* slicer, int ptIndex, int s);
	void getSnakeNormal(int index, double point[3], double normal[3]);
	//void updateReslicer(double point[3], double normal[3], int mode);
	void updateSliceIndicators();
	QString strippedName(const QString &f);

	//! sets up the IO thread for saving the correct file type for the given filename.
	//! \return	true if it succeeds, false if it fails.
	bool setupSaveIO(QString const & f);

	//! sets up the IO thread for loading the correct file type according to the given filename.
	//! \return	true if it succeeds, false if it fails.
	bool setupLoadIO(QString const & f, bool isStack);

	// adds an algorithm to the list of currently running jobs
	void addAlgorithm(iAAlgorithm* thread);

	void setupViewInternal(bool active);

	void setHistogramModality(int modalityIdx);
	void displayHistogram(int modalityIdx);
	int  getCurrentModality() const;
	void initModalities();
	void initVolumeRenderers();

	QByteArray m_beforeMaximizeState;
	QDockWidget * m_whatMaximized;
	int m_pbarMaxVal;

	QFileInfo fileInfo;

	vtkPoints *worldProfilePoints;
	vtkPoints *worldSnakePoints;
	iAParametricSpline *parametricSpline;
	MainWindow * m_mainWnd;

	static const unsigned char RC = 0x01;
	static const unsigned char XY = 0x02;
	static const unsigned char YZ = 0x04;
	static const unsigned char XZ = 0x08;
	static const unsigned char TAB = 0x10;
	static const unsigned char MULTI = 0x1F;

	QString curFile, path;
	int m_position[3];

	iARenderSettings renderSettings;
	iAVolumeSettings volumeSettings;
	iASlicerSettings slicerSettings;
	iAPreferences preferences;

	unsigned char visibility;

	bool m_isSmthMaximized;     //!< whether a single dock widget is currently maximized
	bool isUntitled;            //!< whether current content is saved as a file already
	bool snakeSlicer;           //!< whether snake slicer is enabled
	bool isSliceProfileEnabled; //!< whether slice profile, shown in slices, is enabled
	bool isArbProfileEnabled;   //!< whether arbitrary profile, shown in profile widget
	bool isMagicLensEnabled;    //!< whether magic lens in slicers is enabled
	bool reInitializeRenderWindows; //! whether render windows need to be reinitialized
	bool raycasterInitialized;  //!< whether renderer is already initialized

	vtkSmartPointer<vtkImageData> imageData;		// TODO: remove - use modality data instead!
	vtkPolyData* polyData;
	vtkTransform* axesTransform;
	vtkTransform* slicerTransform;
	vtkAbstractTransform *SlicerYZ_Transform, *SlicerXY_Transform, *SlicerXZ_Transform;
	iARenderer* Raycaster;
	iASlicer * m_slicer[3];
	QSharedPointer<iAProfileProbe> profileProbe;
	QScopedPointer<iAVolumeStack> volumeStack;
	iAIO* ioThread;

	iADiagramFctWidget* m_histogram;
	QSharedPointer<iAPlot> m_histogramPlot;

	//! @{ dock widgets
	iADockWidgetWrapper* m_histogramContainer;
	dlg_imageproperty* imgProperty;
	dlg_volumePlayer* volumePlayer;
	dlg_profile* imgProfile;
	dlg_slicer * m_dlgSlicer[3];
	dlg_modalities * m_dlgModalities;
	dlg_renderer * renderer;
	dlg_logs * m_dlgLog;
	//! @}

	std::vector<iAAlgorithm*> workingAlgorithms;

	QMap<uint, QSharedPointer<iAChannelData> > m_channels;
	uint m_nextChannelID;
	uint m_magicLensChannel;

	int numberOfVolumes;
	int previousIndexOfVolume;

	QList <int> CheckedList;
	iALogger* m_logger;
	QByteArray m_initialLayoutState;
	QString m_layout;
	vtkSmartPointer<vtkImageData> tmpSaveImg;	//< TODO: get rid of this (by introducing smart pointer in iAIO/ iAlgorithm?

	int m_currentModality;
	int m_currentComponent;
	int m_currentHistogramModality;
	bool m_initVolumeRenderers;
	int m_storedModalityNr;		//!< modality nr being stored
};
