/*
 *   HDSPMixer
 *    
 *   Copyright (C) 2003 Thomas Charbonnel (thomas@undata.org)
 *    
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <alsa/asoundlib.h>
#include <alsa/sound/hdsp.h>
#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include "pixmaps.h"
#include "HDSPMixerCard.h"
#include "HDSPMixerWindow.h"
#include "defines.h"

int main(int argc, char **argv)
{
    HDSPMixerWindow *window;
    HDSPMixerCard *hdsp_cards[3];
    char *name;
    int card;
    int cards = 0;
    
    card = -1;
    printf("\nHDSPMixer %s - Copyright (C) 2003 Thomas Charbonnel <thomas@undata.org>\n", VERSION);
    printf("This program comes with ABSOLUTELY NO WARRANTY\n");
    printf("HDSPMixer is free software, see the file COPYING for details\n\n");
    printf("Looking for HDSP cards :\n");
    while (snd_card_next(&card) >= 0) {
	if (card < 0) {
	    break;
	} else {
	    snd_card_get_longname(card, &name);
	    printf("Card %d : %s\n", card, name);
	    if (!strncmp(name, "RME Hammerfall DSP + Multiface", 30)) {
		printf("Multiface found !\n");
		hdsp_cards[cards] = new HDSPMixerCard(Multiface, card);
		cards++;
	    } else if (!strncmp(name, "RME Hammerfall DSP + Digiface", 29)) {
		printf("Digiface found !\n");
		hdsp_cards[cards] = new HDSPMixerCard(Digiface, card);
		cards++;
	    } else if (!strncmp(name, "RME Hammerfall HDSP 9652", 24)) {
		printf("HDSP 9652 found !\n");
		hdsp_cards[cards] = new HDSPMixerCard(H9652, card);
		cards++;
	    } else if (!strncmp(name, "RME Hammerfall HDSP 9632", 24)) {
		printf("HDSP 9632 found !\n");
		hdsp_cards[cards] = new HDSPMixerCard(H9632, card);
		cards++;
	    } else if (!strncmp(name, "RME Hammerfall DSP", 18)) {
		printf("Uninitialized HDSP card found.\nUse hdsploader to upload configuration data to the card.\n");
	    } 
	}
    }
    if (!cards) {
	printf("No Hammerfall DSP card found.\n");
	exit(EXIT_FAILURE);
    }
    for (int i = cards; i < 3; ++i) {
	hdsp_cards[i] = NULL;
    }
    printf("%d Hammerfall DSP %s found.\n", cards, (cards > 1) ? "cards" : "card");
    window = new HDSPMixerWindow(0, 0, hdsp_cards[0]->window_width, hdsp_cards[0]->window_height, "HDSPMixer", hdsp_cards[0], hdsp_cards[1], hdsp_cards[2]);
    Fl::visual(FL_DOUBLE|FL_INDEX);
    window->show(argc, argv);
    return Fl::run();    
}
