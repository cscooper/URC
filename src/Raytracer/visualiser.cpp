/*
 *  visualiser.cpp - visualise the k-factor performance
 *  Copyright (C) 2012  C. S. Cooper, A. Mukunthan
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *  Contact Details: Cooper - andor734@gmail.com
 */

#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cfloat>
#include <vector>

#include "Urc.h"
#include "Raytracer.h"

using namespace std;
using namespace Urc;
using namespace VectorMath;


#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

ALLEGRO_DISPLAY *gDisplay, *gPlotDisplay;
ALLEGRO_BITMAP *gMapBuffer;
Rect gArea, gMapDisplay;
Vector2D gScale;

/** Initialise the visualiser. */
void InitVisualiser() {

	al_init();
	al_install_keyboard();
	al_install_mouse();
	al_init_primitives_addon();
	al_init_image_addon();
	gDisplay = al_create_display( 640, 480 );
	gPlotDisplay = al_create_display( 640, 480 );

}

/** Draw the map inside the given rect. This goes onto a bitmap that is drawn in StartPass. */
void PrepareMap( Rect& area ) {

	gMapBuffer = al_create_bitmap( area.size.x, area.size.y );
	if ( !gMapBuffer )
		throw Exception( "Could not create a bitmap of size %dx%d!", area.size.x, area.size.y );

	al_set_target_bitmap( gMapBuffer );
	al_clear_to_color( al_map_rgb(255,255,255) );

	UrcData *pUrc = UrcData::GetSingleton();
	int linkCount = pUrc->GetSummedLinkCount();
	for ( int link = 0; link < linkCount; link++ ) {

		UrcData::Link *pLink = pUrc->GetSummedLink( link );
		UrcData::Node *pNode1, *pNode2;
		pNode1 = pUrc->GetNode( pLink->nodeAindex );
		pNode2 = pUrc->GetNode( pLink->nodeBindex );

		bool bIn1 = area.PointWithin( pNode1->position );
		bool bIn2 = area.PointWithin( pNode2->position );
		if ( !bIn1 && !bIn2 )
			continue;

		Vector2D start = pNode1->position - area.location;
		Vector2D   end = pNode2->position - area.location;
		al_draw_line( start.x, area.size.y-start.y, end.x, area.size.y-end.y, al_map_rgb(0,0,0), 5 );

	}

	al_set_target_backbuffer( gDisplay );

	gArea = area;
	int w = al_get_bitmap_width(gMapBuffer);
	int h = al_get_bitmap_height(gMapBuffer);
	int newWidth = w;
	int newHeight = h;
	int x, y;
	if ( w < h ) {
		newHeight = 480;
		newWidth = w * (Real)newHeight / (Real)h;
		x = 320 - newWidth/2;
		y = 0;
	} else {
		newWidth = 640;
		newHeight = h * (Real)newWidth / (Real)w;
		x = 0;
		y = 240 - newHeight/2;
	}
	gMapDisplay = Rect( x, y, newWidth, newHeight );
	gScale = Vector2D( newWidth / (Real)w, newHeight / (Real)h );

}

/** Draw the map bitmap. */
void StartPass() {

	al_clear_to_color( al_map_rgb(0,0,0) );

	int w = al_get_bitmap_width(gMapBuffer);
	int h = al_get_bitmap_height(gMapBuffer);

	al_draw_scaled_bitmap( gMapBuffer, 0, 0, w, h, gMapDisplay.location.x, gMapDisplay.location.y, gMapDisplay.size.x, gMapDisplay.size.y, 0 );

}

/** Draw a circular marker with the given colour at the given position. */
void DrawMarker( Vector2D pos, Vector3D col ) {

	al_draw_filled_circle( gMapDisplay.location.x + gScale.x * (pos.x-gArea.location.x), gMapDisplay.location.y + gScale.y * (gArea.size.y-pos.y+gArea.location.y), 5, al_map_rgb_f(col.x,col.y,col.z) );

}

/** Draw a circular marker with the given colour at the given position. */
void DrawRaytrace( const Urc::Raytracer *pRT ) {

	Urc::Raytracer::RayPathComponentSet::const_iterator rayIt;
	for ( AllInVector( rayIt, (*pRT->GetRaySet()) ) ) {

		Vector2D s = rayIt->mLineSegment.mStart;
		Vector2D e = rayIt->mLineSegment.mEnd;

		al_draw_line( gMapDisplay.location.x + gScale.x * (s.x-gArea.location.x),
					  gMapDisplay.location.y + gScale.y * (gArea.size.y-s.y+gArea.location.y), 
					  gMapDisplay.location.x + gScale.x * (e.x-gArea.location.x),
					  gMapDisplay.location.y + gScale.y * (gArea.size.y-e.y+gArea.location.y), 
					  al_map_rgb(0,0,255), 1 );

	}

}

/** Flip the backbuffer. */
void Present() {

	al_flip_display();

}


/** Shows the specified image file in the display view. */
void ShowImage( const char *fname ) {

	ALLEGRO_BITMAP *bmp = al_load_bitmap(fname);
	al_set_target_backbuffer( gPlotDisplay );

	int w, h;
	w = al_get_bitmap_width( bmp );
	h = al_get_bitmap_height( bmp );
	al_draw_scaled_bitmap( bmp, 0, 0, w, h, 0, 0, 640, 480, 0 );

	al_destroy_bitmap(bmp);
	al_flip_display();
	al_set_target_backbuffer( gDisplay );

}

/** Shutdown the visualiser. */
void Shutdown() {

	al_destroy_bitmap( gMapBuffer );
	al_destroy_display( gDisplay );
	al_destroy_display( gPlotDisplay );

}



void ShowRayPowerHistogram( Urc::Raytracer::TraceReport &tReport ) {

	std::ofstream outputStream;
	outputStream.open( "rayPower.tmp" );
	int i = 0;
	for ( ; i < tReport.mRayPowers.size() ; i++ ) {

		outputStream << tReport.mRayPowers[i];
		if ( i+1 < tReport.mRayPowers.size() )
			outputStream << " ";

	}
	outputStream.close();

	std::stringstream str;

	str << "python -c '";
	str << "from matplotlib import pyplot ; ";
	str << "import numpy, os ; ";
	str << "f = open(\"rayPower.tmp\",\"r\") ; ";
	str << "dat = [float(d) for d in f.read().split(\" \")] ; ";
	str << "f.close() ; ";
	str << "h,b = numpy.histogram(dat,10) ; ";
	str << "pyplot.plot( b[0:len(b)-1], h/numpy.trapz(h) ) ;";
	str << "pyplot.vlines( " << tReport.mRayPowerMean << ", 0, max(h/numpy.trapz(h)) ) ;";
	str << "pyplot.xlabel( \"Ray Power (mW)\" ) ; ";
	str << "pyplot.ylabel( \"Probability Density\" ) ; ";
	str << "pyplot.title( \"Ray Power Distribution - Tx: " << tReport.mTransmitterPosition << " Rx: " << tReport.mReceiverPosition << "\" ) ; ";
	str << "pyplot.savefig(\"rayPower.jpg\") ; ";
	str << "os.remove(\"rayPower.tmp\")'";

	system( str.str().c_str() );

	ShowImage( "rayPower.jpg" );

}


void PrintTraceReport( Urc::Raytracer::TraceReport &tReport ) {

	std::cout << "Trace Report for tx: " << tReport.mTransmitterPosition << " and rx: " << tReport.mReceiverPosition << ":\n";
	std::cout << "\tK-Factor: " << tReport.mFactorK << "\n";
	std::cout << "\tSpecular Power: " << tReport.mSpecularPower << "\n";
	std::cout << "\tDiffuse Power: " << tReport.mDiffusePower << "\n";
	std::cout << "\tSpecular Ray Count: " << tReport.mSpecularRayCount << "\n";
	std::cout << "\tDiffuse Ray Count: " << tReport.mDiffuseRayCount << "\n";
	std::cout << "\tRay Power Mean: " << tReport.mRayPowerMean << "\n";
	std::cout << "\tRay Power Variance: " << tReport.mRayPowerVariance << "\n";
	std::cout << "\tRay Power Median: " << tReport.mRayPowerMedian << "\n";
	if ( tReport.mSpecularRayCount > 0 )
		ShowRayPowerHistogram( tReport );

}




int main( int argc, char **ppArgv ) {

	InitVisualiser();
	UrcData *pUrc;

	std::string basename( ppArgv[1] );

	try
	{
		pUrc = new UrcData(
			(basename+".corner.lnk").c_str(),
			(basename+".corner.int").c_str(),
			(basename+".corner.cls").c_str(),
			(basename+".corner.bld").c_str(),
			(basename+".corner.lnm").c_str(),
			15, 0.124378109, 400, 1142.9, pow(10,-11), 0.25, 200
		);

	} catch( Exception &e ) {

		std::cout << "Could not initialise URC. " << e.What() << "\n";
		return -1;

	}

	Rect mapRect = pUrc->GetMapRect();
	PrepareMap( mapRect );

	bool redraw = true;
	bool txPlaced = false;
	bool rxPlaced = false;
	bool buttonLifted = true;

	Vector2D txPos, rxPos;
	Urc::Raytracer *rt = NULL;
	int raycount = 256;
	int cores = 2;
	double gain = 1;
	double raylength = -1;

	bool resizing = false;
	Rect newViewRect;
	Vector2D startNewRect;

	do
	{

		ALLEGRO_KEYBOARD_STATE k;
		al_get_keyboard_state(&k);
		if ( al_key_down( &k, ALLEGRO_KEY_ESCAPE ) )
			break;

		if ( al_key_down( &k, ALLEGRO_KEY_LCTRL ) ) {

			if ( al_key_down( &k, ALLEGRO_KEY_P ) ) {
				std::cout << "Please specify a ray count (current: " << raycount << ") ";
				std::cin >> raycount;
				if ( txPlaced ) {
					delete rt;
					rt = new Raytracer( txPos, raycount, cores );
					if ( raylength != -1 )
						rt->SetRayLength(raylength);
					rt->Execute();
					if ( rxPlaced ) {
						Urc::Raytracer::TraceReport tReport = rt->ComputeK( rxPos, gain );
						if ( tReport.mFactorK >= 0 ) {
							PrintTraceReport( tReport );
						} else {
							std::cout << "Intercepted no rays at receive position: " << rxPos << "\n";
						}
					}
					redraw = true;
				}
			} else if ( al_key_down( &k, ALLEGRO_KEY_L ) ) {
				std::cout << "Please specify a signal power (current: " << pUrc->GetTransmitPower() << ") ";
				Real Ptx;
				std::cin >> Ptx;
				delete pUrc;
				try
				{
					pUrc = new UrcData(
						(basename+".corner.lnk").c_str(),
						(basename+".corner.int").c_str(),
						(basename+".corner.cls").c_str(),
						(basename+".corner.bld").c_str(),
						(basename+".corner.lnm").c_str(),
						15, 0.124378109, Ptx, 1142.9, pow(10,-11), 0.25, 200
					);

				} catch( Exception &e ) {

					std::cout << "Could not initialise URC. " << e.What() << "\n";
					return -1;

				}

				if ( txPlaced ) {
					delete rt;
					rt = new Raytracer( txPos, raycount, cores );
					if ( raylength != -1 )
						rt->SetRayLength(raylength);
					rt->Execute();
					if ( rxPlaced ) {
						Urc::Raytracer::TraceReport tReport = rt->ComputeK( rxPos, gain );
						if ( tReport.mFactorK >= 0 ) {
							PrintTraceReport( tReport );
						} else {
							std::cout << "Intercepted no rays at receive position: " << rxPos << "\n";
						}
					}
					redraw = true;
				}
			} else if ( al_key_down( &k, ALLEGRO_KEY_C ) ) {
				std::cout << "Please specify number of cores to use (current: " << cores << ") ";
				std::cin >> cores;
			} else if ( al_key_down( &k, ALLEGRO_KEY_A ) ) {
				std::cout << "Please specify antenna gain (current: " << gain << ") ";
				std::cin >> gain;
				if ( txPlaced && rxPlaced ) {
					Urc::Raytracer::TraceReport tReport = rt->ComputeK( rxPos, gain );
					PrintTraceReport( tReport );
				}
			} else if ( al_key_down( &k, ALLEGRO_KEY_N ) ) {
				al_destroy_bitmap( gMapBuffer );
				PrepareMap( mapRect );
				redraw = true;
			} else if ( al_key_down( &k, ALLEGRO_KEY_K ) ) {
				
				if ( !txPlaced || !rxPlaced ) {
					std::cout << "Please place the transmitter and receiver.\n";
					continue;
				}

				double incr;
				std::cout << "Please specify distance increment: ";
				std::cin >> incr;
				LineSegment l( rxPos, txPos );
				incr /= l.GetDistance();
				if ( incr <= 0 || incr >= 1 ) {
					std::cout << "Specified distance increment outside length of the line between tx and rx.\n";
					continue;
				}

				std::vector<int> rayCounts;
				std::cout << "Please specify ray counts (Space-delimited. Enter -1 to stop): ";
				while( true ) {
					int tmp;
					std::cin >> tmp;
					if ( tmp == -1 )
						break;
					rayCounts.push_back(tmp);
				};

				std::cout << "Processing. Please wait...\n";

				delete rt;

				std::vector<int>::iterator cntIt;
				std::ofstream outFile;
				for ( AllInVector( cntIt, rayCounts ) ) {

					char tmpfname[100];
					sprintf( tmpfname, "kData-%i.tmp", *cntIt );

					std::cout << "Processing " << *cntIt << " rays.\n";

					outFile.open( tmpfname );
					for ( double t = incr; t < 1; t += incr ) {

						Vector2D p = rxPos + l.GetVector() * t;
						rt = new Raytracer( p, *cntIt, cores );
						if ( raylength != -1 )
							rt->SetRayLength( raylength );
						rt->Execute();
						double kf = rt->ComputeK( rxPos, gain ).mFactorK;
						if ( kf != DBL_MAX )
							outFile << t*l.GetDistance() << " " << kf << "\n";
						delete rt;

					}
					outFile << "-1 -1";	// sort of an EOF
					outFile.close();

				}


				outFile.open("tmp.py");
				outFile << "from matplotlib import pyplot\n";
				outFile << "import numpy, os, csv\n";
				outFile << "fileList = [f for f in os.listdir(\".\") if \"kData\" in f and \"jpg\" not in f]\n";
				outFile << "dat = {}\n";
				outFile << "for fname in fileList:\n";
				outFile << "\traycount = int(fname.strip(\"kData-.tmp\"))\n";
				outFile << "\tf = open(fname,\"r\")\n";
				outFile << "\tr = csv.reader(f,delimiter=\" \")\n";
				outFile << "\tdist = []\n";
				outFile << "\tk = []\n";
				outFile << "\tfor line in r:\n";
				outFile << "\t\tif line[0] != \"-1\":\n";
				outFile << "\t\t\tdist.append( float(line[0]) )\n";
				outFile << "\t\t\tk.append( float(line[1]) )\n";
				outFile << "\tf.close()\n";
				outFile << "\tdat[raycount] = [dist,k]\n";
				outFile << "\tos.remove(fname)\n";
				outFile << "for rc in sorted(dat.keys()):";
				outFile << "\tpyplot.plot( dat[rc][0], dat[rc][1], label=str(rc)+\" rays\" )\n";
				outFile << "pyplot.xlabel( \"Distance (m)\" )\n";
				outFile << "pyplot.ylabel( \"K-Factor\" )\n";
				outFile << "pyplot.legend()\n";
				outFile << "pyplot.ylim(0,100)\n";
				outFile << "pyplot.savefig(\"kData.jpg\")\n";
				outFile << "os.remove(\"tmp.py\")\n";
				outFile.close();

				system( "python tmp.py" );

				std::cout << "Complete.\n";
				ShowImage( "kData.jpg" );

				rt = new Raytracer( txPos, raycount, cores );
				if ( raylength != -1 )
					rt->SetRayLength(raylength);
				rt->Execute();

			} else if ( al_key_down( &k, ALLEGRO_KEY_F ) ) {

				std::cout << "Please specify ray length (-1 to specify transmission range): ";
				std::cin >> raylength;
				if ( txPlaced ) {
					delete rt;
					rt = new Raytracer( txPos, raycount, cores );
					if ( raylength != -1 )
						rt->SetRayLength(raylength);
					rt->Execute();
					if ( rxPlaced ) {
						Urc::Raytracer::TraceReport tReport = rt->ComputeK( rxPos, gain );
						if ( tReport.mFactorK >= 0 ) {
							PrintTraceReport( tReport );
						} else {
							std::cout << "Intercepted no rays at receive position: " << rxPos << "\n";
						}
					}
					redraw = true;
				}

			}

		}

		ALLEGRO_MOUSE_STATE m;
		al_get_mouse_state(&m);
		Vector2D mousePos( al_get_mouse_state_axis(&m, 0), al_get_mouse_state_axis(&m, 1) );
		if ( m.buttons & 1 ) {

			if ( !buttonLifted )
				continue;

			buttonLifted = false;
			if ( al_key_down( &k, ALLEGRO_KEY_R ) ) {

				rxPos.x = gArea.location.x + ( mousePos.x - gMapDisplay.location.x ) / gScale.x;
				rxPos.y = gArea.location.y - ( mousePos.y - gMapDisplay.location.y ) / gScale.y + gArea.size.y;
				rxPlaced = true;

				if ( rt ) {
					Urc::Raytracer::TraceReport tReport = rt->ComputeK( rxPos, gain );
					if ( tReport.mFactorK >= 0 )
						PrintTraceReport( tReport );
					else
						std::cout << "Intercepted no rays at receive position: " << rxPos << "\n";
				}
				redraw = true;

			} else if ( al_key_down( &k, ALLEGRO_KEY_T ) ) {

				txPos.x = gArea.location.x + ( mousePos.x - gMapDisplay.location.x ) / gScale.x;
				txPos.y = gArea.location.y - ( mousePos.y - gMapDisplay.location.y ) / gScale.y + gArea.size.y;
				txPlaced = true;

				delete rt;
				rt = new Raytracer( txPos, raycount, cores );
				if ( raylength != -1 )
					rt->SetRayLength(raylength);
				rt->Execute();
				if ( rxPlaced ) {
					Urc::Raytracer::TraceReport tReport = rt->ComputeK( rxPos, gain );
					if ( tReport.mFactorK >= 0 )
						PrintTraceReport( tReport );
					else
						std::cout << "Intercepted no rays at receive position: " << rxPos << "\n";
				}

				redraw = true;

			} else {

				resizing = true;
				newViewRect.location.x = gArea.location.x + ( mousePos.x - gMapDisplay.location.x ) / gScale.x;
				newViewRect.location.y = gArea.location.y - ( mousePos.y - gMapDisplay.location.y ) / gScale.y + gArea.size.y;
				startNewRect = mousePos;

			}

		} else {

			buttonLifted = true;
			if ( resizing ) {
				
				resizing = false;
				Vector2D p, b;
				p.x = gArea.location.x + ( mousePos.x - gMapDisplay.location.x ) / gScale.x;
				p.y = gArea.location.y - ( mousePos.y - gMapDisplay.location.y ) / gScale.y + gArea.size.y;
				b = newViewRect.location;
				newViewRect.location.x = MIN( p.x, b.x );
				newViewRect.location.y = MIN( p.y, b.y );
				newViewRect.size.x = fabs( p.x - b.x );
				newViewRect.size.y = fabs( p.y - b.y );
				al_destroy_bitmap( gMapBuffer );
				PrepareMap( newViewRect );
				redraw = true;

			}

		}

		if ( redraw || resizing ) {
			
			StartPass();

			if ( txPlaced ) {
				if ( !resizing )
					DrawRaytrace( rt );
				DrawMarker( txPos, Vector3D(1,0,0) );
			}

			if ( rxPlaced )
				DrawMarker( rxPos, Vector3D(0,1,0) );

			if ( resizing ) {
				al_draw_rectangle( MIN(startNewRect.x, mousePos.x), MIN(startNewRect.y, mousePos.y), MAX(startNewRect.x, mousePos.x), MAX(startNewRect.y, mousePos.y), al_map_rgb(1,1,0), 3 );
			}

			Present();

			redraw = false;

		}

	} while( true );

	delete rt;
	Shutdown();

	return 0;

}

