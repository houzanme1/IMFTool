/* Copyright(C) 2016 Björn Stresing, Denis Manthey, Wolfgang Ruppel, Krispin Weiss
 *
 * This program is free software : you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <http://www.gnu.org/licenses/>.
 */
#include "WidgetVideoPreview.h"
#include "WidgetImagePreview.h"
#include "GraphicsWidgetComposition.h" // (k)
#include <QPushButton>
#include <QGridLayout>
#include <QKeyEvent>
#include <QWidget>
#include <QLabel>
#include <QTime>
#include <QCheckBox>
#include "JP2K_Preview.h"
#include "ImfPackage.h"
#include <QThread>
#include <QLineEdit>

//#define DEBUG_JP2K

WidgetVideoPreview::WidgetVideoPreview(QWidget *pParent) : QWidget(pParent) {

	// ############## - create 2 single frame extractors - ###################
	for (int i = 0; i <= 1; i++) {
		decoders[i] = new JP2K_Preview();
		decodingThreads[i] = new QThread();
		decoders[i]->moveToThread(decodingThreads[i]);

		connect(decodingThreads[i], SIGNAL(started()), decoders[i], SLOT(decode()));
		connect(decoders[i], SIGNAL(finished()), decodingThreads[i], SLOT(quit())); // quit thread after work is completed
		connect(decoders[i], SIGNAL(decodingStatus(qint64, QString)), this, SLOT(decodingStatus(qint64,QString))); // decoder -> this
		running[i] = false; // default
	}
	// ####################################################################

	// create player
	player = new JP2K_Player();
	player->setFps(decode_speed); // set default speed

	playerThread = new QThread();
	player->moveToThread(playerThread);
	player->setLayer(decode_layer); // set default layer

	connect(playerThread, SIGNAL(started()), player, SLOT(startPlay()));
	connect(player, SIGNAL(currentPlayerPosition(qint64, bool)), this, SLOT(forwardPlayerPosition(qint64, bool)));
	connect(player, SIGNAL(playTTML()), this, SLOT(getTTML()));
	connect(player, SIGNAL(playbackEnded()), this, SLOT(rPlaybackEnded())); // player -> this

	// create message box
	mpMsgBox = new QMessageBox();
	mpMsgBox->setIcon(QMessageBox::Warning);
	connect(player, SIGNAL(ShowMsgBox(const QString&, int)), this, SLOT(rShowMsgBox(const QString&, int)));

	InitLayout();
}

WidgetVideoPreview::~WidgetVideoPreview(){
	player->~JP2K_Player();
}


void WidgetVideoPreview::InitLayout() {
	
	// layout
	QGridLayout *p_layout = new QGridLayout();
	p_layout->setAlignment(Qt::AlignTop | Qt::AlignCenter);
	p_layout->setContentsMargins(0, 0, 0, 0);
	p_layout->setVerticalSpacing(0);

	// create new gl widget
	mpImagePreview = new WidgetImagePreview(); 
	p_layout->addWidget(mpImagePreview, 1, 0, 1, 5);

	connect(this, SIGNAL(regionOptionsChanged(int)), mpImagePreview, SLOT(regionOptionsChanged(int)));
	connect(player, SIGNAL(showFrame(const QImage&)), mpImagePreview, SLOT(ShowImage(const QImage&))); // player -> glWidget
	connect(decoders[0], SIGNAL(ShowFrame(const QImage&)), mpImagePreview, SLOT(ShowImage(const QImage&))); // decoder -> glWidget
	connect(decoders[1], SIGNAL(ShowFrame(const QImage&)), mpImagePreview, SLOT(ShowImage(const QImage&))); // decoder -> glWidget
	connect(mpImagePreview, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(keyPressEvent(QKeyEvent*)));

	// create menue bar
	menuBar = new QMenuBar(this);
	menuBar->setNativeMenuBar(false);
	menuBar->setObjectName("playerMenu");
	p_layout->addWidget(menuBar, 0, 0, 1, 5); // add menue bar to layout

	// framerates
	menuSpeed = new QMenu(tr("Speed"));
	menuSpeed->setStyleSheet("QMenu::item::checked{background-color:#0080ff;}");


	int i = 0, z = 0;
	while(i != 50){
		if (i >= 30) {
			i += 5;
		}
		else {
			i++;
		}
		z++;
		speeds[z] = new QAction(QString("%1").arg(i));
		speeds[z]->setData(i);
		speeds[z]->setCheckable(true);
		if (i == decode_speed) speeds[z]->setChecked(true);
		menuSpeed->addAction(speeds[z]);
	}

	connect(menuSpeed, SIGNAL(triggered(QAction*)), this, SLOT(rChangeSpeed(QAction*)));
	menuBar->addMenu(menuSpeed);

	// quality layers
	menuQuality = new QMenu(tr("Quality"));
	menuQuality->setStyleSheet("QMenu::item::checked{background-color:#0080ff;}");
	menuBar->addMenu(menuQuality);
	connect(menuQuality, SIGNAL(triggered(QAction*)), this, SLOT(rChangeQuality(QAction*)));

	// image processing
	menuProcessing = new QMenu(tr("Processing"));
	menuBar->addMenu(menuProcessing);
	connect(menuProcessing, SIGNAL(triggered(QAction*)), this, SLOT(rChangeProcessing(QAction*)));

	processing_extract_actions[0] = new QAction(tr("Smooth"));
	processing_extract_actions[0]->setCheckable(true);
	processing_extract_actions[0]->setChecked(true); // default
	processing_extract_actions[0]->setData(0);
	menuProcessing->addAction(processing_extract_actions[0]);

	processing_extract_actions[1] = new QAction(tr("Scaling"));
	processing_extract_actions[1]->setCheckable(true);
	processing_extract_actions[1]->setChecked(true); // default
	processing_extract_actions[1]->setData(1);
	menuProcessing->addAction(processing_extract_actions[1]);

	processing_extract_actions[2] = new QAction(tr("Show subtitles"));
	processing_extract_actions[2]->setCheckable(true);
	processing_extract_actions[2]->setChecked(true); // default
	processing_extract_actions[2]->setData(2);
	menuProcessing->addAction(processing_extract_actions[2]);

	processing_extract_actions[3] = new QAction(tr("real-speed"));
	processing_extract_actions[3]->setCheckable(true);
	processing_extract_actions[3]->setChecked(false); // default
	processing_extract_actions[3]->setData(3);
	menuProcessing->addAction(processing_extract_actions[3]);

	processing_extract_actions[4] = new QAction(tr("convert to REC.709"));
	processing_extract_actions[4]->setCheckable(true);
	processing_extract_actions[4]->setChecked(true); // default
	processing_extract_actions[4]->setData(4);
	menuProcessing->addAction(processing_extract_actions[4]);

	// save image
	processing_extract_actions[5] = new QAction(tr("save image"));
	processing_extract_actions[5]->setData(5);
	menuProcessing->addAction(processing_extract_actions[5]);

	// extract
	processing_extract = menuProcessing->addMenu("extract");
	processing_extract->setObjectName("extractMenu");

	processing_extract_names = new QStringList();
	processing_extract_names->append("top left");
	processing_extract_names->append("top center");
	processing_extract_names->append("top right");
	processing_extract_names->append("center left");
	processing_extract_names->append("center center");
	processing_extract_names->append("center right");
	processing_extract_names->append("bottom left");
	processing_extract_names->append("bottom center");
	processing_extract_names->append("bottom right");

	// add actions
	for (int i = 6; i < (processing_extract_names->length() + 6); i++) {
		processing_extract_actions[i] = new QAction(processing_extract_names->at((i - 6)));
		processing_extract_actions[i]->setCheckable(true);
		processing_extract_actions[i]->setData(i); // set index
		if (i == 0) processing_extract_actions[i]->setChecked(true); // default
		processing_extract->addAction(processing_extract_actions[i]); // add action to menu
	}

	// play button
	mpPlayPauseButton = new QPushButton();
	mpPlayPauseButton->setIcon(QIcon(":/play.png"));
	mpPlayPauseButton->setFlat(true);
	mpPlayPauseButton->setContentsMargins(10, 10, 10, 10);
	p_layout->addWidget(mpPlayPauseButton, 2, 2, 1, 1);
	connect(mpPlayPauseButton, SIGNAL(clicked(bool)), this, SLOT(rPlayPauseButtonClicked(bool)));

	// stop button
	mpStopButton = new QPushButton();
	mpStopButton->setIcon(QIcon(":/stop.png"));
	mpStopButton->setFlat(true);
	mpStopButton->setContentsMargins(10, 10, 10, 10);
	connect(mpStopButton, SIGNAL(clicked(bool)), this, SLOT(stopPlayback(bool)));
	p_layout->addWidget(mpStopButton, 2, 3, 1, 1);

	// info label
	decoding_time = new QLabel();
	decoding_time->setText("");
	decoding_time->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	connect(player, SIGNAL(playerInfo(const QString&)), decoding_time, SLOT(setText(const QString&)));

	// full screen
	menuView = new QMenu(tr("View"));
	menuBar->addMenu(menuView);
	menuView->addAction("Toggle fullscreen");
	connect(menuView, SIGNAL(triggered(QAction*)), this, SLOT(rViewFullScreen(void)));


	p_layout->addWidget(decoding_time, 2, 4, 1, 1);
	
	setLayout(p_layout);
	setFocusPolicy(Qt::ClickFocus);
}

void WidgetVideoPreview::forwardPlayerPosition(qint64 frameNr, bool decode_at_new_pos) {
#ifdef DEBUG_JP2K
	qDebug() << "set frame indicator to:" << frameNr;
#endif
	if(!decode_at_new_pos) setFrameIndicator = true; // ignore next xPosChanged signal!
	emit currentPlayerPosition(frameNr);
}

// stop playback
void WidgetVideoPreview::stopPlayback(bool clicked) {

	player->clean(); // reset player
	if (currentPlaylist.length() > 0) player->setPos(currentPlaylist[0].in, 0, 0);

	if (playerThread->isRunning()) playerThread->quit();

	mpPlayPauseButton->setIcon(QIcon(":/play.png"));
	
	// reset preview decoder
	if (decodingThreads[0]->isRunning()) decodingThreads[0]->quit();
	if (decodingThreads[1]->isRunning()) decodingThreads[1]->quit();

	if(currentPlaylist.length() > 0) decodingFrame = -1; // force preview refresh
	setFrameIndicator = false;
	//if clicked==false --> Button Stop was pressed
	if (!clicked) emit currentPlayerPosition(0); // reset indicator to first frame
}

void WidgetVideoPreview::rViewFullScreen() {
	mpImagePreview->toggleFullScreen();
}

// show message box & handle input
void WidgetVideoPreview::rShowMsgBox(const QString &msg, int new_fps) {

	mpPlayPauseButton->setIcon(QIcon(":/play.png"));

	mpMsgBox->setText(tr("A problem occured during playback!"));
	mpMsgBox->setInformativeText(msg);
	mpMsgBox->setIcon(QMessageBox::Warning);

	if (new_fps == 0) {
		mpMsgBox->setStandardButtons(QMessageBox::Ok);
		mpMsgBox->setDefaultButton(QMessageBox::Ok);
	}
	else {
		mpMsgBox->setStandardButtons(QMessageBox::Apply | QMessageBox::Discard);
		mpMsgBox->setDefaultButton(QMessageBox::Apply);
	}
	
	int ret = mpMsgBox->exec(); // open message box

	if (ret == QMessageBox::Apply && new_fps > 0) {
		speeds[decode_speed]->setChecked(false); // uncheck old value
		decode_speed = new_fps;
		speeds[new_fps]->setChecked(true); // check new value
		player->setFps(new_fps); // set new framreate in player
		playerThread->start(QThread::TimeCriticalPriority); // resume playback
		mpPlayPauseButton->setIcon(QIcon(":/pause.png"));
	}
}

void WidgetVideoPreview::rPlaybackEnded() {
	mpPlayPauseButton->setIcon(QIcon(":/play.png"));
	if(playerThread->isRunning()) playerThread->quit();
}

void WidgetVideoPreview::xPosChanged(const QSharedPointer<AssetMxfTrack> &rAsset, const qint64 &rOffset, const Timecode &rTimecode, const int &playlist_index){

	xSliderFrame = rOffset; // slider is pointing to this frame (within asset)
	xSliderTotal = rTimecode.GetOverallFrames(); // slider is pointing to this frame (in timeline)
	currentAsset = rAsset;
	current_playlist_index = playlist_index;

	if (setFrameIndicator) {
		setFrameIndicator = false;
		return; // ignore this signal (it is a response from setting the player position)
	}
	else if (player->playing) { // pause playback!
		player->playing = false;
		playerThread->quit();
		mpPlayPauseButton->setIcon(QIcon(":/play.png"));
	}

#ifdef DEBUG_JP2K
	qDebug() << "xpos asset:" << xSliderFrame << "total:" << xSliderTotal << "current_playlist_index:" << current_playlist_index;
#endif
	
	if (showTTML) getTTML(); // TTML

	if (decodingFrame != xSliderTotal) {

		player->setPos(xSliderFrame, xSliderTotal, playlist_index); // set current frame in player

		run = (int)now_running;

		if (running[run] == false && !decodingThreads[run]->isRunning()) {
			running[run] = true;

			//emit setVerticalIndicator(const Timecode &rCplTimecode);
#ifdef DEBUG_JP2K
			qDebug() << "start generating preview nr:" << xSliderFrame;
#endif
			decodingFrame = xSliderTotal;

			decoders[run]->asset = currentAsset; // set new asset in current decoder
			//decoders[run]->frameNr = xSliderFrame; // set current frame number in decoder
			if (currentPlaylist.length() == 0) {//Under some race conditions during Outgest currentPlaylist can be empty here.
				qDebug() << "WidgetVideoPreview::xPosChanged called with empty currentPlaylist";
				running[run] = false;
			} else {
				VideoResource vr = currentPlaylist[current_playlist_index];
				decoders[run]->mFrameNr = vr.in + (xSliderFrame - vr.in) % vr.Duration; // set current frame number in decoder
				decodingThreads[run]->start(QThread::HighestPriority); // start decoder
				decoding_time->setText("loading...");
			}
		}
	}
}

void WidgetVideoPreview::decodingStatus(qint64 frameNr,QString status) {

	decoding_time->setText(status);

	running[(int)(now_running)] = false; // free up decoder
	now_running = !now_running; // flip value
	run = (int)now_running;

	// check if current xSlider Position matches last decoded frame
	if (decodingFrame != xSliderTotal && player->playing == false && !decodingThreads[run]->isRunning()){
		
		decodingFrame = xSliderTotal;
#ifdef DEBUG_JP2K
		qDebug() << "now start decoding nr" << xSliderFrame;
#endif
		//TODO
		decoders[run]->asset = currentAsset; // set new asset in current decoder
		//decoders[run]->frameNr = xSliderFrame; // set frame number in decoder
		VideoResource vr = currentPlaylist[current_playlist_index];
		decoders[run]->mFrameNr = vr.in + (xSliderFrame - vr.in) % vr.Duration; // set current frame number in decoder
		player->setPos(xSliderFrame, xSliderTotal, current_playlist_index); // set current frame in player

		decodingThreads[run]->start(QThread::HighestPriority); // start decoder
		decoding_time->setText("loading...");

		// TTML
		if (showTTML) getTTML();
	}

}

void WidgetVideoPreview::rPlayPauseButtonClicked(bool checked) {
	
	mpImagePreview->ttml_regions.clear();

	if (playerThread->isRunning() && player->playing) { // pause playback

		player->playing = false;
		playerThread->quit();
		mpPlayPauseButton->setIcon(QIcon(":/play.png"));

		// set current position in timeline
		if(player->realspeed)
		{
			currentPlayerPosition((int)(player->frame_playing_total_float - player->skip_frames));
		}
		else {
			currentPlayerPosition((int)(player->frame_playing_total_float - 1));
		}
	}
	else if (player->playing) { // player is paused
		playerThread->start(QThread::TimeCriticalPriority); // resume playback
		mpPlayPauseButton->setIcon(QIcon(":/pause.png"));
	}
	else if(currentPlaylist.length() > 0){ // start player
		mpPlayPauseButton->setIcon(QIcon(":/pause.png"));
		playerThread->start(QThread::TimeCriticalPriority);
		emit ttmlChanged(QVector<visibleTTtrack>(), ttml_search_time.elapsed()); // clear subtitle preview
	}else
	{
		decoding_time->setText("Player could not be started - please open a CPL with a video track first!");
	}
}

void WidgetVideoPreview::keyPressEvent(QKeyEvent *pEvent) {

	if(pEvent->key() == Qt::Key_Space) {
		rPlayPauseButtonClicked(false);
	}
	else if(pEvent->key() == Qt::Key_K) {
		if (player->playing) rPlayPauseButtonClicked(false);
	}
	else if(pEvent->key() == Qt::Key_L) {
		if (!player->playing) rPlayPauseButtonClicked(false);
	}
}


void WidgetVideoPreview::Clear() {
	decoding_time->setText("");
	mpImagePreview->Clear();
	emit stopPlayback(true); // stop playback
}

void WidgetVideoPreview::InstallImp() {
	menuQuality->clear(); // clear prev. resolutions from menu
}

void WidgetVideoPreview::UninstallImp() {
	menuQuality->clear(); // clear prev. resolutions from menu
	mpImagePreview->Clear();
}

void WidgetVideoPreview::setPlaylist(QVector<VideoResource> &rPlayList, QVector<TTMLtimelineResource> &rTTMLs) {

	ttmls = &rTTMLs; // set timed text elements
	currentPlaylist = rPlayList; // set playlist
	player->setPlaylist(rPlayList); // set playlist in player
	menuQuality->clear(); // clear prev. resolutions from menu
	current_playlist_index = 0; // set to first item in playlist 

	// create array of TTMLtracks
	int track_index = 0;
	ttml_tracks = QMap<int, QVector<TTMLtimelineResource>>(); // clear all previus tracks
	QMap<int, int> track_indices;
	for (int i = 0; i < ttmls->length(); i++) {
		ttml_tracks[ ttmls->at(i).track_index ].append(ttmls->at(i));
	}

	if (rPlayList.length() > 0) {
		
		// loop playlist & set resolutions
		int width = 0, height = 0, count = -1;
		bool found = false;

		// look for first valid asset
		while (count < (rPlayList.length() - 1) && found == false) {
			count++;
			if (rPlayList.at(count).asset) {
				found = true;
			}
		}

		if (found == false) return; // no valid asset found in timeline

		// use first valid asset to get resolution
		if (rPlayList.at(count).asset->GetMetadata().displayWidth > 0) { // check for displayWidth
			width = rPlayList.at(count).asset->GetMetadata().displayWidth;
			height = rPlayList.at(count).asset->GetMetadata().displayHeight;
		}
		else if (rPlayList.at(count).asset->GetMetadata().storedWidth > 0) { // check for storedWidth
			width = rPlayList.at(count).asset->GetMetadata().storedWidth;
			height = rPlayList.at(count).asset->GetMetadata().storedHeight;
		}

		for (int i = 0; i <= 5; i++) {
			int w = width / pow(2, i);
			int h = height / pow(2, i);
			qualities[i] = new QAction(QString("%1 x %2").arg(w).arg(h)); // create new action
			qualities[i]->setData(i);
			qualities[i]->setCheckable(true);
			if (i == decode_layer) qualities[i]->setChecked(true); // default
			menuQuality->addAction(qualities[i]);
		}

		// load first picture in preview
		decoding_time->setText("loading...");
		currentAsset = rPlayList[0].asset;
		now_running = !now_running; // use same decoder (relevant frame is still set)
		decoders[0]->asset = currentAsset; // set new asset in decoder 1
		decoders[1]->asset = currentAsset; // set new asset in decoder 2
		decoders[(int)(now_running)]->mFrameNr = rPlayList[0].in; // set first frame
		decodingThreads[(int)(now_running)]->start(QThread::HighestPriority); // start decoder (again)

		if (showTTML) getTTML(); // look for TTML

		player->setPos(rPlayList.at(0).in, xSliderTotal, current_playlist_index); // set current frame in player
	}
	else {
		xSliderFrame = 0;
		xSliderTotal = 0;
		decodingFrame = 0;
		decoding_time->setText("No/empty playlist!");
		Clear(); // stop player & clear preview
	}
}

void WidgetVideoPreview::rChangeSpeed(QAction *action) {
	
	int decode_speed_index = decode_speed;
	if (decode_speed > 30) decode_speed_index = 30 + (decode_speed - 30) / 5;

	if (decode_speed != action->data().value<int>()) { // value has changed

		speeds[decode_speed_index]->setChecked(false); // uncheck 'old' value
		decode_speed = action->data().value<int>();
		decode_speed_index = decode_speed;
	
		bool was_playing = player->playing;
		if (playerThread->isRunning()) playerThread->quit();
		player->setFps(decode_speed);
		player->clean();

		if (was_playing) {
			player->playing = true;
			playerThread->start(QThread::TimeCriticalPriority); // resume playback
		}
	}
	else {
		if (decode_speed > 30) decode_speed_index = (decode_speed - 30) / 5;
		speeds[decode_speed_index]->setChecked(true); // check again!
	}
}

void WidgetVideoPreview::rChangeQuality(QAction *action) {

	if (decode_layer != action->data().value<int>()) {
		qualities[decode_layer]->setChecked(false); // uncheck 'old' layer

		decode_layer = action->data().value<int>();
		player->setLayer(decode_layer);
		decoders[0]->setLayer(decode_layer);
		decoders[1]->setLayer(decode_layer);

		// reload the same frame again (if player is not playing)
		if (!player->playing) {
			now_running = !now_running; // use same decoder (relevant frame is still set)
			decodingThreads[(int)now_running]->start(QThread::HighestPriority); // start decoder (again)
			decoding_time->setText("loading...");
		}
	}
	else {
		qualities[decode_layer]->setChecked(true); // check again!
	}
}

void WidgetVideoPreview::rChangeProcessing(QAction *action) {
	
	int nr = action->data().value<int>();
	bool player_playing = player->playing;

	switch (nr) {
	case 0: // smoothing
		if (action->isChecked()) {
			mpImagePreview->setSmoothing(true); // smoothing is on
		}
		else {
			mpImagePreview->setSmoothing(false); // smoothing is off
		}
		break;
	case 1: // scaling
		if (action->isChecked()) {
			mpImagePreview->setScaling(true); // scaling is on
			processing_extract_actions[processing_extract_action]->setChecked(false); // uncheck 'old' option
		}
		else {
			mpImagePreview->setScaling(false); // scaling is off
			processing_extract_actions[10]->setChecked(true); // center center is ON
			processing_extract_action = 10;
			mpImagePreview->setExtract(4);
		}
		break;
	case 2:
		player->show_subtitles = action->isChecked();
		if (!player->show_subtitles) {
			emit ttmlChanged(QVector<visibleTTtrack>(), ttml_search_time.elapsed()); // clear subtitle preview
		}
		break;
	case 3: // real speed

		if(playerThread->isRunning()) playerThread->quit();
		player->realspeed = action->isChecked();
		player->clean();

		if (player_playing) {
			player->playing = true;
			playerThread->start(QThread::TimeCriticalPriority); // resume playback
		}

		break;
	case 4: // color space conversion
		decoders[0]->convert_to_709 = action->isChecked(); // set in decoder 0
		decoders[1]->convert_to_709 = action->isChecked(); // set in decoder 1
		player->convert_to_709(action->isChecked()); // set in player

		if (!player_playing) {
			// reload current preview
			decoding_time->setText("loading...");
			now_running = !now_running; // use same decoder (relevant frame is still set)
			decodingThreads[(int)(now_running)]->start(QThread::HighestPriority); // start decoder (again)
		}

		break;
	case 5: // save image
		mpImagePreview->saveImage();
		break;
	}

	if (nr >= 6) {
		if (action->isChecked()) {
			processing_extract_actions[1]->setChecked(false); // scaling is off
			mpImagePreview->setScaling(false); // scaling is off
		}
		else {
			processing_extract_actions[1]->setChecked(true); // turn scaling back on
			mpImagePreview->setScaling(true); // scaling is on
		}
		
		// extract action!
		if(processing_extract_action != nr) processing_extract_actions[processing_extract_action]->setChecked(false); // uncheck 'old' option
		processing_extract_action = nr;
		mpImagePreview->setExtract((nr - 6));
	}
}

void WidgetVideoPreview::getTTML() {

	float time = ((float)xSliderTotal / CPLEditRate);

	ttml_search_time.start();
	current_tt = QVector<visibleTTtrack>();
	mpImagePreview->ttml_regions.clear();

	float frac_sec;
	double seconds, rel_time;
	int rel_frame;
	QString h, m, s, f;

	// loop tracks
	QMap<int, QVector<TTMLtimelineResource>>::iterator x;
	for (x = ttml_tracks.begin(); x != ttml_tracks.end(); ++x) { // i.key(), i.value()

		// loop segments in track
		for (int i = 0; i < x.value().length(); i++) {

			TTMLtimelineResource resource = x.value().at(i);

			float beg_rounded = ceil(qRound(resource.timeline_in * CPLEditRate * 100.f) / 100.f);
			float end_rounded = ceil(qRound(resource.timeline_out * CPLEditRate * 100.f) / 100.f);

			// loop tt elements within segment
			if (xSliderTotal >= beg_rounded && xSliderTotal < end_rounded) {
				// visible segment found -> search inside segment

				visibleTTtrack tt;
				tt.resource = resource;

				if (resource.RepeatCount > 0) { // RepeatCount != 0
					if (modf((time - resource.timeline_in) / (resource.out - resource.in), &seconds) > 0.99) {
						rel_time = resource.in;
					}
					else {
						rel_time = modf((time - resource.timeline_in) / (resource.out - resource.in), &seconds) * (resource.out - resource.in) + resource.in;
					}
					rel_frame = (qRound((time - resource.timeline_in)*CPLEditRate) % qRound((resource.out - resource.in) * CPLEditRate)) + resource.in * CPLEditRate;
				}
				else { // no repeating elements
					rel_time = ((time - resource.timeline_in) + resource.in);
					rel_frame = qRound(((time - resource.timeline_in) + resource.in)*CPLEditRate);
				}

				frac_sec = modf(rel_time, &seconds);

				h = QString("%1").arg((int)(seconds / 3600.0f), 2, 10, QChar('0')); // hours
				m = QString("%1").arg((int)(seconds / 60.0f), 2, 10, QChar('0')); // minutes
				s = QString("%1").arg((int)(seconds) % 60, 2, 10, QChar('0')); // seconds
				tt.formatted_time = QString("%1 : %2 : %3").arg(h).arg(m).arg(s); // ttml timecode
				tt.fractional_frames = QString::number(qRound(frac_sec * resource.frameRate * (float)100) / (float)100, 'f', 2);

				for (int z = 0; z < resource.items.length(); z++) {

					TTMLelem ttelem = resource.items.at(z);
					float beg_rounded = ceil(qRound(ttelem.beg * CPLEditRate * 100.f) / 100.f);
					float end_rounded = ceil(qRound(ttelem.end * CPLEditRate * 100.f) / 100.f);

					if (rel_frame >= beg_rounded && rel_frame < end_rounded) {

						// add visible element
						tt.elements.append(ttelem);

						if (ttelem.type == 1) {
							ttelem.region.bgImage = ttelem.bgImage; // image
						}

						// append region
						mpImagePreview->ttml_regions.append(ttelem.region);
					}
				}
				current_tt.append(tt); // add visible track
			}
		}
	}

	emit ttmlChanged(current_tt, ttml_search_time.elapsed());
}

void WidgetVideoPreview::rPrevNextSubClicked(bool direction) {

	// calculate frame indicator time
	float next = -1, prev = -1, item_begin = 0;
	int repeat = 1;

	// loop tracks
	QMap<int, QVector<TTMLtimelineResource>>::iterator x;
	for (x = ttml_tracks.begin(); x != ttml_tracks.end(); ++x) { // i.key(), i.value()
		// loop resources within track
		for (int i = 0; i < x.value().count(); i++) {
			TTMLtimelineResource resource = x.value().at(i);

			if (resource.RepeatCount > 1) {
				repeat = resource.RepeatCount;
			}
			else {
				repeat = 1; // default
			}
#ifdef DEBUG_JP2K
					qDebug() << "resource.in" << resource.in << "resource.timeline_in" << resource.timeline_in
							<< "resource.out" << resource.out << "resource.timeline_out" << resource.timeline_out;
#endif

			// loop repeated elements
			for (int ii = 0; ii < repeat; ii++) {
				// loop items within resource
				for (int iii = 0; iii < resource.items.length(); iii++) {
					item_begin = ceil(qRound((resource.items.at(iii).beg - resource.in + resource.timeline_in) * CPLEditRate * 100.f) / 100.f) + qRound((resource.out - resource.in)*ii * CPLEditRate);
					if (item_begin < 0) item_begin = 0;
#ifdef DEBUG_JP2K
					qDebug() << "item" << item_begin << "resource.items.at(iii).beg" << resource.items.at(iii).beg;
#endif

					if (prev == -1) {
						prev = item_begin; // initialize prev
#ifdef DEBUG_JP2K
						qDebug() << "init prev.";
#endif
					}

					if (next == -1 && item_begin > xSliderTotal) { // make sure there is at least one frame difference to current frame
						next = item_begin; // initialize next
#ifdef DEBUG_JP2K
						qDebug() << "init next.";
#endif
					}

					if (item_begin < xSliderTotal && item_begin > prev) { // make sure there is at least one frame difference to current frame
						prev = item_begin;
#ifdef DEBUG_JP2K
						qDebug() << "update prev.";
#endif
					}
				}
			}
		}
	}
#ifdef DEBUG_JP2K
	qDebug() << "CPL fps:" << CPLEditRate << "frame indicator" << xSliderTotal << "prev" << prev << "next" << next;
#endif

	if (next == -1) { // no next found, use last time
		next = item_begin;
	}

	if (direction == true) { 
		emit currentPlayerPosition(next); // next
#ifdef DEBUG_JP2K
		qDebug() << "set NEXT to qRound of:" << (next * CPLEditRate);
#endif
	}
	else {
		emit currentPlayerPosition(prev); // previous
#ifdef DEBUG_JP2K
		qDebug() << "set PREV to qRound of:" << (prev * CPLEditRate);
#endif
	}
}
