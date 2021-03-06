/***********************************************************************
** Copyright (C) 2010, 20011 Rakesh Achanta <rakeshvar at gmail.com>
** Copyright (C) 2010 Aleksey Sytchev <194145 at gmail.com>
**
** This file is part of Cowboser project
** http://code.google.com/p/cowboxer/
** All rights reserved.
**
** This file may be used under the terms of the GNU GENERAL PUBLIC
** LICENSE Version 3, 29 June 2007 as published by the Free Software
** Foundation and appearing in the file LICENSE included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
***********************************************************************/

#include <QDir>
#include <QtGui>
#include <QFileInfo>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
  init();
  setCurrentFile(tr(""));
}

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow(const QString &fileName)
{
  init();
  loadFile(fileName);
}

void MainWindow::init()
{
  isUntitled = true;

  cowboxer = new cowBoxer(this);
  cowboxer->setMinimumSize(253, 133);   // size of "image not loaded" image

  scrollArea = new cowScrollArea;
  scrollArea->setBackgroundRole(QPalette::Base);
  scrollArea->setWidget(cowboxer);
  scrollArea->setCowboxer(cowboxer);

  setCentralWidget(scrollArea);
  //setWindowIcon(QIcon(":/images/images/icon.png"));
  setWindowIcon(QIcon(":/images/images/cowboxer.svg"));

  setActions();
  readSettings();   //set the window to the last opened position
  (void)statusBar();
}

void MainWindow::setActions()
{
  // action to create new windows
  newAct = new QAction(tr("&New"), this);
  newAct->setIcon(QIcon(":/images/images/new.png"));
  newAct->setShortcuts(QKeySequence::New);
  newAct->setStatusTip(tr("Create a new empty window"));
  connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

  // action to open existing box file
  openBoxAct = new QAction(tr("&Open box file..."), this);
  openBoxAct->setIcon(QIcon(":/images/images/openBox.png"));
  openBoxAct->setShortcuts(QKeySequence::Open);
  openBoxAct->setStatusTip(tr("Open an existing box file"));
  connect(openBoxAct, SIGNAL(triggered()), this, SLOT(openBox()));

  // action to save box file
  saveBoxAct = new QAction(tr("&Save box file"), this);
  saveBoxAct->setIcon(QIcon(":/images/images/save.png"));
  saveBoxAct->setShortcuts(QKeySequence::Save);
  saveBoxAct->setStatusTip(tr("Save the box file"));
  connect(saveBoxAct, SIGNAL(triggered()), this, SLOT(save()));

  // action to save box file
  saveAsBoxAct = new QAction(tr("Save box file &as..."), this);
  saveAsBoxAct->setIcon(QIcon(":/images/images/save-as.png"));
  saveAsBoxAct->setShortcuts(QKeySequence::SaveAs);
  saveAsBoxAct->setStatusTip(tr("Save the box file as..."));
  connect(saveAsBoxAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  // action to open image, that concurs to the box file
  openImageAct = new QAction(tr("Open &image file..."), this);
  openImageAct->setIcon(QIcon(":/images/images/openImage.png"));
  openImageAct->setShortcuts(QKeySequence::Italic);   // Italic means alias of Ctrl+I in all systems. Actually there is no any actions with font type.
  openImageAct->setStatusTip(tr("Open an image file"));
  connect(openImageAct, SIGNAL(triggered()), this, SLOT(openImage()));

  for (int i = 0; i < MaxRecentFiles; ++i)
    {
      recentFileActs[i] = new QAction(this);
      recentFileActs[i]->setVisible(false);
      connect(recentFileActs[i], SIGNAL(triggered()),
              this, SLOT(openRecentFile()));
    }

  // action exit
  exitAct = new QAction(tr("E&xit"), this);
  exitAct->setIcon(QIcon(":/images/images/exit.png"));
  exitAct->setShortcuts(QKeySequence::Quit);
  exitAct->setStatusTip(tr("Exit the application"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  // about act
  aboutAct = new QAction(tr("&About"), this);
  aboutAct->setIcon(QIcon(":/images/images/help.png"));
  aboutAct->setShortcuts(QKeySequence::HelpContents);
  aboutAct->setStatusTip(tr("How to use CowBoxer"));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAct = new QAction(tr("About &Qt"), this);
  connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  // add actions to the file menu
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newAct);
  fileMenu->addAction(openBoxAct);
  fileMenu->addAction(openImageAct);
  separatorAct = fileMenu->addSeparator();
  for (int i = 0; i < MaxRecentFiles; ++i)
    fileMenu->addAction(recentFileActs[i]);
  fileMenu->addSeparator();
  fileMenu->addAction(saveBoxAct);
  fileMenu->addAction(saveAsBoxAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);
  updateRecentFileActions();

  menuBar()->addSeparator();

  // add actions to the about menu
  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);

  // add actions to the toolbars
  fileToolBar = new QToolBar(tr("File"));
  fileToolBar->addAction(newAct);
  fileToolBar->addAction(openBoxAct);
  fileToolBar->addAction(saveBoxAct);
  fileToolBar->addAction(saveAsBoxAct);
  fileToolBar->addAction(openImageAct);
  fileToolBar->addAction(aboutAct);
  fileToolBar->addAction(exitAct);  
  addToolBar(fileToolBar);

  connect(cowboxer, SIGNAL(currentBoxChanged(int, int)),
          scrollArea, SLOT(ensurePositionVisibility(int, int)));
}

void MainWindow::openBox(const QString &path)
{
  QString fileName;
  QString last_path;
  QSettings settings;

  if (path.isNull())
    {
      last_path = settings.value("last_path").toString();   // try to get path from settings
      fileName = QFileDialog::getOpenFileName(this, tr("Open Tesseract box file"),
                                              last_path,
                                              tr("Tesseract box files (*.box);;All files (*.*)"));
    }
  else
    fileName = path;

  if (!fileName.isEmpty())
    {
      MainWindow *existing = findMainWindow(fileName);
      if (existing)     // if this file already opened
        {
          existing->show();
          existing->raise();
          existing->activateWindow();
          return;
        }
      if (isUntitled && cowboxer->isEmpty() && !isWindowModified())     // if there is no opened document before
        {
          loadFile(fileName);
        }
      else       // if we have opened document then we create the new one
        {
          MainWindow *other;
          other = new MainWindow(fileName);
          if (other->isUntitled)     // something goes wrong? delete
            {
              delete other;
              return;
            }
          other->move(x() + 40, y() + 40);
          other->show();
        }
    }
}

void MainWindow::openImage(const QString &path)
{
  QSettings settings;
  QString last_path;

  if (path.isNull())
    last_path = settings.value("last_path").toString();    // try to get path from settings
  else
    last_path = path;

  QString imageFileName = QFileDialog::getOpenFileName(NULL, tr("Open image file"),
                                                       last_path,
                                                       tr("Image files (*.bmp *.png *.jpeg *.jpg *.tif *.tiff);;Tiff files (*.tif *.tiff);;All files (*.*)"));

  if (!imageFileName.isEmpty())     // if we have file
    {
      cowboxer->setImageFile(imageFileName);
    }
}

void MainWindow::openRecentFile()
{
  QAction *action = qobject_cast<QAction *>(sender());

  if (action)
    {
    if (maybeSave())
      {
        writeSettings();
        loadFile(action->data().toString());
      }
    else
      {
        loadFile(action->data().toString());
      }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (maybeSave())
    {
      writeSettings();
      event->accept();
    }
  else
    {
      event->ignore();
    }
}

void MainWindow::newFile()
{
  MainWindow *other = new MainWindow;

  other->move(x() + 40, y() + 40);
  other->show();
}

bool MainWindow::save()
{
  if (isUntitled)
    {
      return saveAs();
    }
  else
    {
      return saveFile(curFile);
    }
}

bool MainWindow::saveAs()
{
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), curFile,
                                                  tr("Tesseract box files (*.box);;All files (*.*)"));

  if (fileName.isEmpty())
    return false;

  return saveFile(fileName);
}

void MainWindow::about()
{
  QMessageBox::about(this, tr("About CowBoxer"),
                     tr(
                       "<h1>CowBoxer 1.01</h1>"
                       "The <b>CowBoxer</b> is the tool for training the \"Google Tesseract OCR\" "
                       "<h2>Controls</h2>"
                       "<p><b>Arrow key</b> - Move box<br>"
                       "<b>Arrow key + Shift</b> - Fast moving<br>"
                       "<b>Arrow key + Ctrl</b> - Change box size<br>"
                       "<b>Arrow key + Ctrl + Shift</b> - Fast box size changing</p>"
                       "<p><b>Return</b> - Select next box<br>"
                       "<b>Shift + Return</b> - Select previous box</p>"
                       "<p><b>Ctrl + Shift + Del</b> - Delete current box along with the string<br>"
                       "<b>Ctrl + Del</b> - Delete current box only (string transfers to next box)<br>"
                       "<b>Shift + Del</b> - Delete current box's string only (next box's moves here)<br>"
                       "<b>Del</b> - Clear current box's string</p>"
                       "<p><b>Ctrl + Shift + Ins</b> - Insert new box & string (\"A\")<br>"
                       "<b>Ctrl + Ins</b> - Insert new box only (next box's string moves to this box)<br>"
                       "<b>Shift + Ins</b> - Insert new string only (current string moves to next box)</p>"
                       "<p><b>Tab</b> - Slice the current box vertically and trim.<br>"
                       "<i>On loading a box file, boxes containing a \"~\" are sliced automatically.<br></i>"
                       "<b>Ctrl + Tab</b> - Slice the current box horizontally and trim.<br>"
                       "<b>Home</b> - Slice the current box horizontally and then vertically. <br>"
                       "<i>On loading, if box file is empty this is done automatically.</i></p>"
                       "<p><b>Backspace</b> - Backspace string<br>"
                       "<b>Other keys</b> - Input string for the current box</p>"
                       "<h2>Project page</h2>"
                       "<a href=\"http://code.google.com/p/cowboxer/\">CowBoxer homepage at Google Code</a><br>"
                       "CowBoxer is available under the <a href=\"http://www.gnu.org/licenses/gpl.html\">GNU General Public License v3</a><br>"
                       "<h2>Author(s)</h2>"
                       "<b>Aleksey Sytchev</b> - 194145 (at) gmail.com<br>"
                       "<b>Rakeshvara Rao</b> - rakeshvar (at) gmail.com<br>"
                       "<h2>Contributor</h2>"
                       "<b>Zdenko Podobny> - zdenop (at) gmail.com<br>"));
}

void MainWindow::readSettings()
{
  QSettings settings("Tesseract-OCR", "CowBoxer");
  QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
  QSize size = settings.value("size", QSize(400, 400)).toSize();

  move(pos);
  resize(size);
}

void MainWindow::writeSettings()
{
  QSettings settings("Tesseract-OCR", "CowBoxer");

  settings.setValue("pos", pos());
  settings.setValue("size", size());
}

bool MainWindow::maybeSave()
{
  if (cowboxer->isModified())
    {
      QMessageBox::StandardButton ret;
      ret = QMessageBox::warning(this, tr("CowBoxer"), tr("The boxfile has been modified.\n"
                                                          "Do you want to save your changes?"),
                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
      if (ret == QMessageBox::Save)
        return save();
      else if (ret == QMessageBox::Cancel)
        return false;
    }

  return true;
}

void MainWindow::loadFile(const QString &fileName)
{
  QFile file(fileName);

  if (!file.open(QFile::ReadOnly | QFile::Text))
    {
      QMessageBox::warning(this, tr("CowBoxer"),
                           tr("Cannot read file %1:\n%2.")
                           .arg(fileName)
                           .arg(file.errorString()));
      return;
    }
  file.close();   // [I don't need problems with you. Get off! You are useless now. ] I'll call you later, maybe. Bye.

  QString imageFileName;
  QString box_file = file.fileName();   // remember this path to acces to the image file
  QString image_base = box_file.replace(box_file.lastIndexOf(tr(".box")), 4, "");   // remove file extension

  QStringList image_files;
  image_files.append(image_base + ".tif");  // we prefare tif/tiff
  image_files.append(image_base + ".tiff");
  image_files.append(image_base + ".png");
  image_files.append(image_base + ".bmp");
  image_files.append(image_base + ".jpg");  // very bad idea to use jpeg for training but it can be useful
  image_files.append(image_base + ".jpeg");
  foreach (const QString& image_file, image_files)
    {
      if (QFile::exists(image_file))
        {
          imageFileName = image_file;
          break;
        }
    }

  if (imageFileName.isEmpty())
    {
      openImage(QFileInfo(box_file).absolutePath()); // Does image file is not found yet? Get me its name, and i'll get it!
    }
  else
    {
      cowboxer->setImageFile(imageFileName);   // we have got it! load and run out of function
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  bool loadedSuccessful = cowboxer->loadBoxFile(fileName);
  QApplication::restoreOverrideCursor();
  if (!loadedSuccessful)    // if i can't read the file
    {
      QMessageBox::warning(this, tr("CowBoxer"),
                           tr("Cannot read file %1:\nAre you sure that you don't read the binary file?")
                           .arg(fileName));
      return;
    }
  isUntitled = false;
  setCurrentFile(fileName);
  statusBar()->showMessage(tr("File loaded"), 2000);

  // save path of open box file
  QSettings settings;
  QString filePath = QFileInfo(fileName).absolutePath();
  settings.setValue("last_path", filePath);
}

bool MainWindow::saveFile(const QString &fileName)
{
  QFile file(fileName);

  if (!file.open(QFile::WriteOnly | QFile::Text))
    {
      QMessageBox::warning(this, tr("CowBoxer"),
                           tr("Cannot write file %1:\n%2.")
                           .arg(fileName)
                           .arg(file.errorString()));
      return false;
    }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  bool savedSuccessful = cowboxer->saveBoxFile(fileName);
  QApplication::restoreOverrideCursor();
  if (!savedSuccessful)    // if i can't save the file
    {
      QMessageBox::warning(this, tr("CowBoxer"),
                           tr("Cannot save file %1:\nThat's bad :( Maybe you want to rewrite the readonly file.")
                           .arg(fileName));
      return false;
    }

  setCurrentFile(fileName);
  statusBar()->showMessage(tr("File saved"), 2000);
  isUntitled = false;
  return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
  if (!fileName.isEmpty())
    {
      curFile = fileName;
      setWindowFilePath(curFile);

      QSettings settings;
      QStringList files = settings.value("recentFileList").toStringList();
      files.removeAll(fileName);
      files.prepend(fileName);
      while (files.size() > MaxRecentFiles)
        files.removeLast();

      settings.setValue("recentFileList", files);

      foreach (QWidget *widget, QApplication::topLevelWidgets())
        {
          MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
          if (mainWin)
            mainWin->updateRecentFileActions();
        }
    }

  //static int sequenceNumber = 1;

  isUntitled = fileName.isEmpty();
  if (isUntitled)
    {
      //curFile = tr("box%1.box").arg(sequenceNumber++);
      curFile = tr("Untitled");
    }
  else
    {
      curFile = QFileInfo(fileName).canonicalFilePath();
    }

  setWindowTitle(tr("%1 - %2").arg(strippedName(curFile))
                 .arg(tr("CowBoxer")));
}

void MainWindow::updateRecentFileActions()
{
  QSettings settings;
  QStringList files = settings.value("recentFileList").toStringList();

  int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

  for (int i = 0; i < numRecentFiles; ++i)
    {
      QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
      recentFileActs[i]->setText(text);
      recentFileActs[i]->setData(files[i]);
      recentFileActs[i]->setVisible(true);
    }
  for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
    recentFileActs[j]->setVisible(false);

  separatorAct->setVisible(numRecentFiles > 0);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}

MainWindow *MainWindow::findMainWindow(const QString &fileName)
{
  QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

  foreach (QWidget * widget, qApp->topLevelWidgets())
    {
      MainWindow *mainWin = qobject_cast<MainWindow *>(widget);

      if (mainWin && mainWin->curFile == canonicalFilePath)
        return mainWin;
    }
  return 0;
}
