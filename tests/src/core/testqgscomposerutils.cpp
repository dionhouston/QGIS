/***************************************************************************
                         testqgscomposerutils.cpp
                         -----------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerutils.h"
#include "qgscomposition.h"
#include "qgscompositionchecker.h"
#include <QObject>
#include <QtTest>

class TestQgsComposerUtils: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void drawArrowHead(); //test drawing an arrow head
    void angle(); //test angle utility function
    void rotate(); //test rotation helper function
    void largestRotatedRect(); //test largest rotated rect helper function

  private:
    bool renderCheck( QString testName, QImage &image, int mismatchCount = 0 );
    QgsComposition* mComposition;
    QgsMapSettings mMapSettings;
    QString mReport;

};

void TestQgsComposerUtils::initTestCase()
{
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mReport = "<h1>Composer Utils Tests</h1>\n";
}

void TestQgsComposerUtils::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsComposerUtils::init()
{

}

void TestQgsComposerUtils::cleanup()
{

}

void TestQgsComposerUtils::drawArrowHead()
{
  //test drawing with no painter
  QgsComposerUtils::drawArrowHead( 0, 100, 100, 90, 30 );

  //test painting on to image
  QImage testImage = QImage( 250, 250, QImage::Format_RGB32 );
  testImage.fill( qRgb( 152, 219, 249 ) );
  QPainter testPainter;
  testPainter.begin( &testImage );
  QgsComposerUtils::drawArrowHead( &testPainter, 100, 100, 45, 30 );
  testPainter.end();
  QVERIFY( renderCheck( "composerutils_drawarrowhead", testImage, 40 ) );
}

void TestQgsComposerUtils::angle()
{
  //test angle with zero length line
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 1, 1 ) ), 0.0 );

  //test angles to different quadrants
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 1, 2 ) ), 180.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 2, 2 ) ), 135.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 2, 1 ) ), 90.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 2, 0 ) ), 45.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 1, 0 ) ), 0.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 0, 0 ) ), 315.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 0, 1 ) ), 270.0 );
  QCOMPARE( QgsComposerUtils::angle( QPointF( 1, 1 ), QPointF( 0, 2 ) ), 225.0 );
}

void TestQgsComposerUtils::rotate()
{
  // pairs of lines from before -> expected after position and angle to rotate
  QList< QPair< QLineF, double > > testVals;
  testVals << qMakePair( QLineF( 0, 1, 0, 1 ), 0.0 );
  testVals << qMakePair( QLineF( 0, 1, -1, 0 ), 90.0 );
  testVals << qMakePair( QLineF( 0, 1, 0, -1 ), 180.0 );
  testVals << qMakePair( QLineF( 0, 1, 1, 0 ), 270.0 );
  testVals << qMakePair( QLineF( 0, 1, 0, 1 ), 360.0 );

  //test rotate helper function
  QList< QPair< QLineF, double > >::const_iterator it = testVals.constBegin();
  for ( ; it != testVals.constEnd(); ++it )
  {
    double x = ( *it ).first.x1();
    double y = ( *it ).first.y1();
    QgsComposerUtils::rotate(( *it ).second, x, y );
    QVERIFY( qgsDoubleNear( x, ( *it ).first.x2() ) );
    QVERIFY( qgsDoubleNear( y, ( *it ).first.y2() ) );
  }
}

void TestQgsComposerUtils::largestRotatedRect()
{
  QRectF wideRect = QRectF( 0, 0, 2, 1 );
  QRectF highRect = QRectF( 0, 0, 1, 2 );
  QRectF bounds = QRectF( 0, 0, 4, 2 );

  //simple cases
  //0 rotation
  QRectF result = QgsComposerUtils::largestRotatedRectWithinBounds( wideRect, bounds, 0 );
  QCOMPARE( result, QRectF( 0, 0, 4, 2 ) );
  result = QgsComposerUtils::largestRotatedRectWithinBounds( highRect, bounds, 0 );
  QCOMPARE( result, QRectF( 1.5, 0, 1, 2 ) );
  // 90 rotation
  result = QgsComposerUtils::largestRotatedRectWithinBounds( wideRect, bounds, 90 );
  QCOMPARE( result, QRectF( 1.5, 0, 2, 1 ) );
  result = QgsComposerUtils::largestRotatedRectWithinBounds( highRect, bounds, 90 );
  QCOMPARE( result, QRectF( 0, 0, 2, 4 ) );
  // 180 rotation
  result = QgsComposerUtils::largestRotatedRectWithinBounds( wideRect, bounds, 180 );
  QCOMPARE( result, QRectF( 0, 0, 4, 2 ) );
  result = QgsComposerUtils::largestRotatedRectWithinBounds( highRect, bounds, 0 );
  QCOMPARE( result, QRectF( 1.5, 0, 1, 2 ) );
  // 270 rotation
  result = QgsComposerUtils::largestRotatedRectWithinBounds( wideRect, bounds, 270 );
  QCOMPARE( result, QRectF( 1.5, 0, 2, 1 ) );
  result = QgsComposerUtils::largestRotatedRectWithinBounds( highRect, bounds, 270 );
  QCOMPARE( result, QRectF( 0, 0, 2, 4 ) );
  //360 rotation
  result = QgsComposerUtils::largestRotatedRectWithinBounds( wideRect, bounds, 360 );
  QCOMPARE( result, QRectF( 0, 0, 4, 2 ) );
  result = QgsComposerUtils::largestRotatedRectWithinBounds( highRect, bounds, 360 );
  QCOMPARE( result, QRectF( 1.5, 0, 1, 2 ) );

  //full test, run through a circle in 10 degree increments
  for ( double rotation = 10; rotation < 360; rotation += 10 )
  {
    result = QgsComposerUtils::largestRotatedRectWithinBounds( wideRect, bounds, rotation );
    QTransform t;
    t.rotate( rotation );
    QRectF rotatedRectBounds = t.mapRect( result );
    //one of the rotated rects dimensions must equal the bounding rectangles dimensions (ie, it has been constrained by one dimension)
    //and the other dimension must be less than or equal to bounds dimension
    QVERIFY(( qAbs( rotatedRectBounds.width() - bounds.width() ) < 0.0001 && ( rotatedRectBounds.height() <= bounds.height() ) )
            || ( qAbs( rotatedRectBounds.height() - bounds.height() ) < 0.0001 && ( rotatedRectBounds.width() <= bounds.width() ) ) );
  }
  //and again for the high rectangle
  for ( double rotation = 10; rotation < 360; rotation += 10 )
  {
    result = QgsComposerUtils::largestRotatedRectWithinBounds( highRect, bounds, rotation );
    QTransform t;
    t.rotate( rotation );
    QRectF rotatedRectBounds = t.mapRect( result );
    //one of the rotated rects dimensions must equal the bounding rectangles dimensions (ie, it has been constrained by one dimension)
    //and the other dimension must be less than or equal to bounds dimension
    QVERIFY(( qAbs( rotatedRectBounds.width() - bounds.width() ) < 0.0001 && ( rotatedRectBounds.height() <= bounds.height() ) )
            || ( qAbs( rotatedRectBounds.height() - bounds.height() ) < 0.0001 && ( rotatedRectBounds.width() <= bounds.width() ) ) );
  }
}

bool TestQgsComposerUtils::renderCheck( QString testName, QImage &image, int mismatchCount )
{
  mReport += "<h2>" + testName + "</h2>\n";
  QString myTmpDir = QDir::tempPath() + QDir::separator() ;
  QString myFileName = myTmpDir + testName + ".png";
  image.save( myFileName, "PNG" );
  QgsRenderChecker myChecker;
  myChecker.setControlName( "expected_" + testName );
  myChecker.setRenderedImage( myFileName );
  bool myResultFlag = myChecker.compareImages( testName, mismatchCount );
  mReport += myChecker.report();
  return myResultFlag;
}

QTEST_MAIN( TestQgsComposerUtils )
#include "moc_testqgscomposerutils.cxx"