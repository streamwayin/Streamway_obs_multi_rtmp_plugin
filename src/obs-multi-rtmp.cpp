#include "pch.h"

#include <list>
#include <regex>
#include <filesystem>

#include "push-widget.h"

#include "output-config.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#define ConfigSection "obs-multi-rtmp"

static class GlobalServiceImpl : public GlobalService {
public:
	bool RunInUIThread(std::function<void()> task) override
	{
		if (uiThread_ == nullptr)
			return false;
		QMetaObject::invokeMethod(
			uiThread_, [func = std::move(task)]() { func(); });
		return true;
	}

	QThread *uiThread_ = nullptr;
} s_service;

GlobalService &GetGlobalService()
{
	return s_service;
}

class MultiOutputWidget : public QDockWidget {
	int dockLocation_;
	bool dockVisible_;
	bool reopenShown_;

	QLabel *codeLabel_;
	QLineEdit *uidLineEdit_;
    QLineEdit *keyLineEdit_;
	QPushButton *verifyButton_;
	QNetworkAccessManager *networkManager_;
	QNetworkReply *networkReply;
	QJsonDocument jsonDoc;
	QJsonObject jsonObj;

public:
	MultiOutputWidget(QWidget *parent = 0)
		: QDockWidget(parent), reopenShown_(false)
	{
		setWindowTitle(obs_module_text("Title"));
		setFeatures(QDockWidget::DockWidgetFloatable |
			    QDockWidget::DockWidgetMovable);

		// save dock location
		QObject::connect(this, &QDockWidget::dockLocationChanged,
				 [this](Qt::DockWidgetArea area) {
					 dockLocation_ = area;
				 });

		scroll_ = new QScrollArea(this);
		scroll_->move(0, 22);

		container_ = new QWidget(this);
		layout_ = new QVBoxLayout(container_);
		layout_->setAlignment(Qt::AlignmentFlag::AlignTop);

		// Add a label and text box for entering the uid
		codeLabel_ = new QLabel("Uid", container_);
		layout_->addWidget(codeLabel_);

		uidLineEdit_ = new QLineEdit(container_);
		layout_->addWidget(uidLineEdit_);

        // Add a label and text box for entering the code
		codeLabel_ = new QLabel("API Key", container_);
		layout_->addWidget(codeLabel_);

		keyLineEdit_ = new QLineEdit(container_);
		layout_->addWidget(keyLineEdit_);

		// Add a button for verification
		verifyButton_ = new QPushButton("Verify", container_);
		layout_->addWidget(verifyButton_);

		QObject::connect(verifyButton_, &QPushButton::clicked, [this]() {
			// Get the code entered by the user
			QString uid = uidLineEdit_->text();
            QString key = keyLineEdit_->text();

			// Construct the request
			QNetworkRequest request(QUrl("http://localhost:8000"));
			request.setHeader(QNetworkRequest::ContentTypeHeader,
					  "application/json");

			// Create a QNetworkAccessManager for making the HTTP request
			networkManager_ = new QNetworkAccessManager(this);

			// Construct the POST data with the Refer Code
			// QByteArray postData;
			// postData.append("ReferCode=" + code.toUtf8());
			QJsonObject obj;
			obj["uid"] = uid;
			obj["api_key"] = key;
			QJsonDocument doc(obj);
			QByteArray postData = doc.toJson();

			// Send the POST request
			networkReply = networkManager_->post(request, postData);

			// Connect to the finished signal to handle the response
			QObject::connect(networkReply, &QNetworkReply::finished, [this]() {
				if (networkReply->error() ==
				    QNetworkReply::NoError) {
					// Successful response received

					// Read the response data
					QByteArray responseData =
						networkReply->readAll();

					// TODO: Parse the response data to extract the token
					// Assuming the response contains a JSON object with a "token" field
					jsonDoc = QJsonDocument::fromJson(
						responseData);
					jsonObj = jsonDoc.object();

					if (jsonObj.contains("token")) {
						QString token =
							jsonObj["token"]
								.toString();

						// You have a valid token
						// You can use 'token' as needed
						qDebug() << "Token: " << token;
						handleSuccessfulLogin(token);
					} else {
						// No token found in the response, indicating an invalid code
						qDebug() << "Invalid token";
						QLabel *codeLabela_ = new QLabel(
							"Invalid token:",
							container_);
						layout_->addWidget(codeLabela_);
					}
				} else {
					// Handle the network error here
					qDebug() << "Network error: "
						 << networkReply->errorString();
					QLabel *codeLabelc_ = new QLabel(
						networkReply->errorString(),
						container_);
					layout_->addWidget(codeLabelc_);
				}

				// Clean up the network resources
				networkReply->deleteLater();
			});
		});

		// init widget
		auto addButton = new QPushButton(
			obs_module_text("Btn.NewTarget"), container_);
		QObject::connect(addButton, &QPushButton::clicked, [this]() {
			auto &global = GlobalMultiOutputConfig();
			auto newid = GenerateId(global);
			auto target = std::make_shared<OutputTargetConfig>();
			target->id = newid;
			global.targets.emplace_back(target);
			auto pushwidget = createPushWidget(newid, container_);
			itemLayout_->addWidget(pushwidget);
			if (pushwidget->ShowEditDlg())
				SaveConfig();
			else {
				auto it = std::find_if(global.targets.begin(),
						       global.targets.end(),
						       [newid](auto &x) {
							       return x->id ==
								      newid;
						       });
				if (it != global.targets.end())
					global.targets.erase(it);
				delete pushwidget;
			}
		});
		layout_->addWidget(addButton);

		// start all, stop all
		auto allBtnContainer = new QWidget(this);
		auto allBtnLayout = new QHBoxLayout();
		auto startAllButton = new QPushButton(
			obs_module_text("Btn.StartAll"), allBtnContainer);
		allBtnLayout->addWidget(startAllButton);
		auto stopAllButton = new QPushButton(
			obs_module_text("Btn.StopAll"), allBtnContainer);
		allBtnLayout->addWidget(stopAllButton);
		allBtnContainer->setLayout(allBtnLayout);
		layout_->addWidget(allBtnContainer);

		QObject::connect(startAllButton, &QPushButton::clicked,
				 [this]() {
					 for (auto x : GetAllPushWidgets())
						 x->StartStreaming();
				 });
		QObject::connect(stopAllButton, &QPushButton::clicked,
				 [this]() {
					 for (auto x : GetAllPushWidgets())
						 x->StopStreaming();
				 });

		// load config
		itemLayout_ = new QVBoxLayout(this);
		LoadConfig();
		layout_->addLayout(itemLayout_);

		// donate
		if (std::string(
			    "\xe5\xa4\x9a\xe8\xb7\xaf\xe6\x8e\xa8\xe6\xb5\x81") ==
		    obs_module_text("Title")) {
			auto cr = new QWidget(container_);
			auto innerLayout = new QGridLayout(cr);
			innerLayout->setAlignment(Qt::AlignmentFlag::AlignLeft);

			auto label = new QLabel(
				u8"免费领红包或投喂支持插件作者。", cr);
			innerLayout->addWidget(label, 0, 0, 1, 2);
			innerLayout->setColumnStretch(0, 4);
			auto label2 = new QLabel(u8"作者：雷鸣", cr);
			innerLayout->addWidget(label2, 1, 0, 1, 1);
			auto btnFeed = new QPushButton(u8"支持", cr);
			innerLayout->addWidget(btnFeed, 1, 1, 1, 1);

			QObject::connect(btnFeed, &QPushButton::clicked, [this]() {
				const char redbagpng[] =
					"iVBORw0KGgoAAAANSUhEUgAAAJgAAACXAQMAAADTWgC3AAAABlBMVEUAAAD///+l2Z/dAAAAAWJLR0Q"
					"AiAUdSAAAAAlwSFlzAAAuIwAALiMBeKU/dgAAAWtJREFUSMe1lk2OgzAMhY1YZJkj5CbkYkggcTG4SY"
					"6QZRaonmcHqs7PYtTaVVWSLxJu7JfnEP/+0H9ZIaKRA0aZz4QJJXuGQFsJO9HU104H1ihuTENl4IS12"
					"YmVcFSa4unJuE2xZV69mOav5XrX6nRgMi6Ii+3Nr9p4k8m7w5OtmOVbzw8ZrOmbxs0Y/ktENMlfnQnx"
					"NweG2vB1ZrZCPoyPfmbwXWSPiz1DvIrHwOHgFQsQtTkrmG6McRvqUu4aGbM28Cm1wRM62HtOP2DwkFF"
					"ypKVoU/dEa8Y9rtaFJLg5EzscoSfBKMWgZ8aY9bj4EQ1jo9GDIR68kKukMCF/6pPWTPW0R9XulVNzpj"
					"6Z5ZzMpOZrzvRElPC49Awx2LOi3k7aP+akhnL1AEMmPYphvtqeGD032TPt5zB2kQBq5Mgo9hrl7lceT"
					"MQsEkD80YH1O9xRw9Vzn/cSQ6Y6EK1JH3nVxvss/GCf3L3/YF97Nxv6vuoIAwAAAABJRU5ErkJggg==";
				const char alipaypng[] =
					"iVBORw0KGgoAAAANSUhEUgAAALsAAAC4AQMAAACByg+HAAAABlBMVEUAAAD///+l2Z/dAAAAAWJLR0Q"
					"AiAUdSAAAAAlwSFlzAAAuIwAALiMBeKU/dgAAAVtJREFUWMPNmEGugzAMRM0qx+CmIbkpx8gK1zM2/U"
					"L66w4RogqvC5dhxqbm/6/TfgEuwzr8PNwnjhP7pgYNxR1r97XPhUvxjUsPztj0hkJ7O7c4m/V3gCg36"
					"r5Q68uA7SHtaC8BlBbP2T4awFNzEaANOtSt4+EPEciEiKJn2eCZJSIQ5XbU5yFt+HNUfIgBNqFo6Grh"
					"BDxttIQcoL5ICrsTbdAYWgBPIltxK7uVRaeLQToTFRMbb+JoYoAHjsG63UWXtFIQm2w/mV9x9vodUnC"
					"vCWfuiA+oqwaLH7Al8qL/aS4GA9I6zkz/bbFBSsHVKi++Dftg89YC53DDq8iLTJCSVgcq6DlMZGRkzm"
					"pBtW2jRRH3flU3UIIacWBLduv0gxzwltGTaUtOFS4HVSJHnBp35jvAsbJnV/RbewWgtLVyAlODkjZSd"
					"eMrkNfsIwX3awYfuLLEaGKg/OvlAz+wXVruSNSgAAAAAElFTkSuQmCC";
				const char wechatpng[] =
					"iVBORw0KGgoAAAANSUhEUgAAAK8AAACtAQMAAAD8lL09AAAABlBMVEUAAAD///+l2Z/dAAAAAWJLR0Q"
					"AiAUdSAAAAAlwSFlzAAAuIwAALiMBeKU/dgAAAbpJREFUSMe9l0GugzAMRI26yDJHyE3Si1WiEhcrN8"
					"kRWLJA8Z9xaNUv/eUfUIXgqQtn4pkY87+ubv+Cd8O1T3g2y4unzveqxXf3BniUeLKa3XcxrnZr+32zg"
					"iXPDvy0S/C04WYo4jpsKCK97FH2K3DovfqzoJzAX9sgwtFVNS/tvH03mwYPs6zb7PjDrf22lAZT6ugq"
					"wwtaCzLYWLwO13yUUQl3N70yDSTFWOqCJbORsdloLYguxrSNgxyWhl3Z0l2LjWaZUE541qaRFUqM342"
					"5sDTYdY5EZE1KjHU/z25+0UV3yi/GE4LeubHe0V9QYFZjEOhN70QOmp0JocSdGT82Nh+D7nrcQoHkDO"
					"GOcmLnhXil3vAsy5nRWhS9SjGkhmMiddHIMO6580oMu5a0xpAC0aOSOHd0mC+jodjNnhyVqDH1Tj3Ts"
					"0hEm3CGv7dYhDkdQGr0cOJxxquMM02GI44g+tKiqwwZVd4HugjHfOKxeEYvQ9jVOKawzrRnQkQSf4Yz"
					"EeasiWHhwfYNF8WkIscc985pKCaGSzA9S6eGAmpMvRlFzonBLZ6qFp8nyhxSM/IP+3x2arDwc/kHxnM"
					"tm62qBBUAAAAASUVORK5CYII=";
				auto donateWnd = new QDialog();
				donateWnd->setWindowTitle(u8"赞助");
				auto redbagBtn = new QPushButton(
					u8"支付宝领红包", donateWnd);
				auto redbagQr = new QLabel(donateWnd);
				auto redbagQrBmp =
					QPixmap::fromImage(QImage::fromData(
						QByteArray::fromBase64(
							QByteArray::fromRawData(
								redbagpng,
								sizeof(redbagpng) -
									1)),
						"png"));
				redbagQr->setPixmap(redbagQrBmp);
				auto aliBtn =
					new QPushButton(u8"支付宝", donateWnd);
				auto aliQr = new QLabel(donateWnd);
				auto aliQrBmp =
					QPixmap::fromImage(QImage::fromData(
						QByteArray::fromBase64(
							QByteArray::fromRawData(
								alipaypng,
								sizeof(alipaypng) -
									1)),
						"png"));
				aliQr->setPixmap(aliQrBmp);
				auto weBtn =
					new QPushButton(u8"微信", donateWnd);
				auto weQr = new QLabel(donateWnd);
				auto weQrBmp = QPixmap::fromImage(QImage::fromData(
					QByteArray::fromBase64(
						QByteArray::fromRawData(
							wechatpng,
							sizeof(wechatpng) - 1)),
					"png"));
				weQr->setPixmap(weQrBmp);
				auto layout = new QGridLayout(donateWnd);
				layout->addWidget(redbagBtn, 0, 0);
				layout->addWidget(aliBtn, 0, 1);
				layout->addWidget(weBtn, 0, 2);
				layout->addWidget(redbagQr, 1, 0, 1, 3);
				layout->addWidget(aliQr, 1, 0, 1, 3);
				layout->addWidget(weQr, 1, 0, 1, 3);
				aliQr->setVisible(false);
				weQr->setVisible(false);
				QObject::connect(
					redbagBtn, &QPushButton::clicked,
					[redbagQr, aliQr, weQr]() {
						redbagQr->setVisible(true);
						aliQr->setVisible(false);
						weQr->setVisible(false);
					});
				QObject::connect(
					aliBtn, &QPushButton::clicked,
					[redbagQr, aliQr, weQr]() {
						redbagQr->setVisible(false);
						aliQr->setVisible(true);
						weQr->setVisible(false);
					});
				QObject::connect(
					weBtn, &QPushButton::clicked,
					[redbagQr, aliQr, weQr]() {
						redbagQr->setVisible(false);
						aliQr->setVisible(false);
						weQr->setVisible(true);
					});
				donateWnd->setLayout(layout);
				donateWnd->exec();
			});

			layout_->addWidget(cr);
		} else {
			auto label = new QLabel(
				u8"<p>This plugin is provided for free. <br>Author: SoraYuki (<a href=\"https://paypal.me/sorayuki0\">donate</a>) </p>",
				container_);
			label->setTextFormat(Qt::RichText);
			label->setTextInteractionFlags(
				Qt::TextBrowserInteraction);
			label->setOpenExternalLinks(true);
			layout_->addWidget(label);
		}

		scroll_->setWidgetResizable(true);
		scroll_->setWidget(container_);

		setLayout(layout_);

		resize(200, 400);
	}


void handleSuccessfulLogin(const QString& token) {
    QMainWindow* newUi = new QMainWindow;
    newUi->resize(500, 500);
	newUi->setMaximumSize(500 , 500);
    QWidget* centralWidget = new QWidget;
    QVBoxLayout* newUiLayout = new QVBoxLayout(centralWidget);

    QLabel* successLabel = new QLabel("Success Login");
   

    QNetworkRequest request(QUrl("http://localhost:8000/v1/broadcasts/upcoming"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString authHeader = "Bearer " + token;
    request.setRawHeader("Authorization", authHeader.toUtf8());

    QNetworkAccessManager networkManager;
    QNetworkReply* networkReply = networkManager.get(request);

    // Create an event loop to wait for the network request to finish
    QEventLoop eventLoop;
    QObject::connect(&networkManager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);

    eventLoop.exec(); // This will block until the network request finishes

    if (networkReply->error() == QNetworkReply::NoError) {
        QByteArray responseData = networkReply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonArray jsonArray = jsonDoc.array();

		

        QLabel* tokenLabelSize = new QLabel("Size is: " + QString::number(jsonArray.size()));
        newUiLayout->addWidget(tokenLabelSize);


		// Create a scroll area and set up a widget to contain the items
QScrollArea* scrollArea = new QScrollArea;
QWidget* scrollWidget = new QWidget;
scrollArea->setWidgetResizable(true);
scrollArea->setWidget(scrollWidget);

// Create a layout for the widget inside the scroll area
QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

for (const QJsonValue& jsonValue : jsonArray) {
    if (jsonValue.isObject()) {
        QJsonObject jsonObject = jsonValue.toObject();
        if (jsonObject.contains("title")) {
            // Create a custom widget to represent each item
            QWidget* itemWidget = new QWidget;
            QVBoxLayout* itemLayout = new QVBoxLayout(itemWidget);

			// Create a group for title and scheduledTime
            QGroupBox* titleScheduledGroup = new QGroupBox;
            QVBoxLayout* titleScheduledLayout = new QVBoxLayout(titleScheduledGroup);

			// Retrieve the image URL from the jsonObject
			QString imageUrl = jsonObject["thumbnail"].toString();
            // Show jsonObject thumbnail field in an image (Assuming you have a QLabel for this)
            QLabel* thumbnailLabel = new QLabel;
            
			
			thumbnailLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			QNetworkAccessManager networkManager;
			// Create a QUrl object with the image URL
			QUrl url(imageUrl);

			QNetworkRequest request(url);
			QNetworkReply* reply = networkManager.get(request);

			// Handle the image loading asynchronously
			QObject::connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();
        QImage thumbnail;
        thumbnail.loadFromData(imageData); // Load image data into the QImage instance
        QPixmap pixmap = QPixmap::fromImage(thumbnail);
        thumbnailLabel->setPixmap(pixmap);
    } else {
        qDebug() << "Failed to load image:" << reply->errorString();
    }
    reply->deleteLater();
});


			QLabel* gfgf = new QLabel("scheduledTime");
            titleScheduledLayout->addWidget(gfgf);
			titleScheduledLayout->addWidget(thumbnailLabel); // Add the image label
            // Show title from jsonObject
            QString title = jsonObject["title"].toString();
            QLabel* titleLabel = new QLabel(title);
            titleScheduledLayout->addWidget(titleLabel);

			QPushButton* SelectButton = new QPushButton("Select");
			QObject::connect(SelectButton, &QPushButton::clicked, [this]() {
			auto &global = GlobalMultiOutputConfig();
			auto newid = GenerateId(global);
			auto target = std::make_shared<OutputTargetConfig>();
			target->id = newid;
			target->name = "YourNameHere";
			target->serviceParam = {
    								{"server", "rtpm://ams.streamway.in"},
    								{"key", "132456"},
								   };
			global.targets.emplace_back(target);
			auto pushwidget = createPushWidget(newid, container_);
			itemLayout_->addWidget(pushwidget);
			// if (pushwidget->ShowSaveDlg())
				SaveConfig();
		// 	else {
		// 		auto it = std::find_if(global.targets.begin(),
		// 				       global.targets.end(),
		// 				       [newid](auto &x) {
		// 					       return x->id ==
		// 						      newid;
		// 				       });
		// 		if (it != global.targets.end())
		// 			global.targets.erase(it);
		// 		delete pushwidget;
		// 	}
		});
			titleScheduledLayout->addWidget(SelectButton);



            // Show scheduledTime from jsonObject
            QString scheduledTime = jsonObject["scheduledTime"].toString();
            QLabel* timeLabel = new QLabel(scheduledTime);
            titleScheduledLayout->addWidget(timeLabel);

			// Add the title and scheduledTime group to the item layout
            itemLayout->addWidget(titleScheduledGroup);



            // Add the custom widget to the scroll layout
            scrollLayout->addWidget(itemWidget);
        }
    }
}


// Create a horizontal layout for Add and Cancel buttons
QHBoxLayout* buttonLayout = new QHBoxLayout;

// Create Cancel and Add buttons outside of the loop
QPushButton* cancelButton = new QPushButton("Cancel");
QPushButton* addButton = new QPushButton("Add");

// Add the scroll area to your main layout or window
newUiLayout->addWidget(scrollArea);
// Add Cancel and Add buttons to the layout or wherever appropriate
buttonLayout->addWidget(cancelButton);
buttonLayout->addWidget(addButton);

// Create a container widget for the button layout
QWidget* buttonContainer = new QWidget;
buttonContainer->setLayout(buttonLayout);
newUiLayout->addWidget(buttonContainer);
// Connect signals and slots for Cancel and Add buttons
// QObject::connect(cancelButton, &QPushButton::clicked, window, &QWidget::close);
        
    } else {
        qDebug() << "Network error: " << networkReply->errorString();
        // Handle network errors here
    }

    networkReply->deleteLater();

    newUiLayout->addWidget(successLabel);
  

    newUi->setCentralWidget(centralWidget);
    newUi->show();
}



	void visibleToggled(bool visible)
	{
		dockVisible_ = visible;

		if (visible == false && reopenShown_ == false &&
		    !config_has_user_value(obs_frontend_get_global_config(),
					   ConfigSection, "DockVisible")) {
			reopenShown_ = true;
			QMessageBox(QMessageBox::Icon::Information,
				    obs_module_text("Notice.Title"),
				    obs_module_text("Notice.Reopen"),
				    QMessageBox::StandardButton::Ok, this)
				.exec();
		}
	}

	bool event(QEvent *event) override
	{
		if (event->type() == QEvent::Resize) {
			scroll_->resize(width(), height() - 22);
		}
		return QDockWidget::event(event);
	}

	std::vector<PushWidget *> GetAllPushWidgets()
	{
		std::vector<PushWidget *> result;
		for (auto &c : container_->children()) {
			if (c->objectName() == "push-widget") {
				auto w = dynamic_cast<PushWidget *>(c);
				result.push_back(w);
			}
		}
		return result;
	}

	void SaveConfig() { SaveMultiOutputConfig(); }


	void LoadConfig()
	{
		for (auto x : GetAllPushWidgets()) {
			delete x;
		}
		GlobalMultiOutputConfig() = {};

		if (LoadMultiOutputConfig() == false) {
			ImportLegacyMultiOutputConfig();
		}

		for (auto x : GlobalMultiOutputConfig().targets) {
			auto pushwidget = createPushWidget(x->id, container_);
			itemLayout_->addWidget(pushwidget);
		}
	}

private:
	QWidget *container_ = 0;
	QScrollArea *scroll_ = 0;
	QVBoxLayout *itemLayout_ = 0;
	QVBoxLayout *layout_ = 0;
};

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-multi-rtmp", "en-US")
OBS_MODULE_AUTHOR("雷鳴 (@sorayukinoyume)")

bool obs_module_load()
{
	// check obs version
	if (obs_get_version() < MAKE_SEMANTIC_VERSION(29, 1, 0)) {
		blog(LOG_ERROR, TAG "Request OBS 29.1 or higher.");
		return false;
	}

	auto mainwin = (QMainWindow *)obs_frontend_get_main_window();
	if (mainwin == nullptr)
		return false;
	QMetaObject::invokeMethod(mainwin, []() {
		s_service.uiThread_ = QThread::currentThread();
	});

	auto dock = new MultiOutputWidget(mainwin);
	dock->setObjectName("obs-multi-rtmp-dock");
	auto action = (QAction *)obs_frontend_add_dock(dock);
	QObject::connect(action, &QAction::toggled, dock,
			 &MultiOutputWidget::visibleToggled);

	obs_frontend_add_event_callback(
		[](enum obs_frontend_event event, void *private_data) {
			auto mainwin =
				static_cast<MultiOutputWidget *>(private_data);

			for (auto x : mainwin->GetAllPushWidgets())
				x->OnOBSEvent(event);

			if (event ==
			    obs_frontend_event::OBS_FRONTEND_EVENT_EXIT) {
				mainwin->SaveConfig();
			} else if (event ==
				   obs_frontend_event::
					   OBS_FRONTEND_EVENT_PROFILE_CHANGED) {
				mainwin->LoadConfig();
			}
		},
		dock);

	return true;
}

const char *obs_module_description(void)
{
	return "Multiple RTMP Output Plugin";
}
