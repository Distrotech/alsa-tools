/*****************************************************************************
   envy24control.c - Env24 chipset (ICE1712) control utility
   Copyright (C) 2000 by Jaroslav Kysela <perex@suse.cz>

   (2003/03/22) Changed to hbox/vbox layout.
   Copyright (C) 2003 by Søren Wedel Nielsen
   
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "envy24control.h"
#define _GNU_SOURCE
#include <getopt.h>

int input_channels, output_channels, spdif_channels;
ice1712_eeprom_t card_eeprom;
snd_ctl_t *ctl;

GtkWidget *window;

GtkWidget *mixer_mix_drawing;
GtkWidget *mixer_clear_peaks_button;
GtkWidget *mixer_drawing[20];
GtkObject *mixer_adj[20][2];
GtkWidget *mixer_vscale[20][2];
GtkWidget *mixer_mute_toggle[20][2];
GtkWidget *mixer_stereo_toggle[20];

GtkWidget *router_radio[10][12];

//GtkWidget *hw_master_clock_xtal_radio;
GtkWidget *hw_master_clock_xtal_22050;
GtkWidget *hw_master_clock_xtal_32000;
GtkWidget *hw_master_clock_xtal_44100;
GtkWidget *hw_master_clock_xtal_48000;
GtkWidget *hw_master_clock_xtal_88200;
GtkWidget *hw_master_clock_xtal_96000;
GtkWidget *hw_master_clock_spdif_radio;
GtkWidget *hw_master_clock_word_radio;
GtkWidget *hw_master_clock_status_label;
GtkWidget *hw_master_clock_actual_rate_label;

GtkWidget *hw_clock_state_label;
GtkWidget *hw_clock_state_locked;
GtkWidget *hw_clock_state_reset;

GtkWidget *hw_rate_locking_check;
GtkWidget *hw_rate_reset_check;

GtkObject *hw_volume_change_adj;
GtkWidget *hw_volume_change_spin;

GtkWidget *hw_spdif_profi_nonaudio_radio;
GtkWidget *hw_spdif_profi_audio_radio;

GtkWidget *hw_profi_stream_stereo_radio;
GtkWidget *hw_profi_stream_notid_radio;

GtkWidget *hw_profi_emphasis_none_radio;
GtkWidget *hw_profi_emphasis_5015_radio;
GtkWidget *hw_profi_emphasis_ccitt_radio;
GtkWidget *hw_profi_emphasis_notid_radio;

GtkWidget *hw_consumer_copyright_on_radio;
GtkWidget *hw_consumer_copyright_off_radio;

GtkWidget *hw_consumer_copy_1st_radio;
GtkWidget *hw_consumer_copy_original_radio;

GtkWidget *hw_consumer_emphasis_none_radio;
GtkWidget *hw_consumer_emphasis_5015_radio;

GtkWidget *hw_consumer_category_dat_radio;
GtkWidget *hw_consumer_category_pcm_radio;
GtkWidget *hw_consumer_category_cd_radio;
GtkWidget *hw_consumer_category_general_radio;

GtkWidget *hw_spdif_professional_radio;
GtkWidget *hw_spdif_consumer_radio;
GtkWidget *hw_spdif_output_notebook;

GtkWidget *hw_spdif_input_coaxial_radio;
GtkWidget *hw_spdif_input_optical_radio;

GtkWidget *input_interface_internal;
GtkWidget *input_interface_front_input;
GtkWidget *input_interface_rear_input;
GtkWidget *input_interface_wavetable;

GtkWidget *hw_breakbox_led_on_radio;
GtkWidget *hw_breakbox_led_off_radio;

GtkWidget *hw_phono_input_on_radio;
GtkWidget *hw_phono_input_off_radio;

GtkWidget *hw_spdif_switch_on_radio;
GtkWidget *hw_spdif_switch_off_radio;

GtkObject *av_dac_volume_adj[10];
GtkObject *av_adc_volume_adj[10];
GtkObject *av_ipga_volume_adj[10];
GtkLabel *av_dac_volume_label[10];
GtkLabel *av_adc_volume_label[10];
GtkLabel *av_ipga_volume_label[10];
GtkWidget *av_dac_sense_radio[10][4];
GtkWidget *av_adc_sense_radio[10][4];


static void create_mixer_frame(GtkWidget *box, int stream)
{
	GtkWidget *vbox;
	GtkWidget *vbox1;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkObject *adj;
	GtkWidget *vscale;
	GtkWidget *drawing;
	GtkWidget *label;
	GtkWidget *toggle;
	char str[64], drawname[32];

	if (stream <= 10) {
		sprintf(str, "PCM Out %i", stream);
	} else if ((card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE) && (stream == 11)) {
		sprintf(str, "CD In L");
	} else if ((card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE) && (stream == 12)) {
		sprintf(str, "CD In R");
	} else if ((card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE) && (stream == 13)) {
		sprintf(str, "Line In L");
	} else if ((card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE) && (stream == 14)) {
		sprintf(str, "Line In R");
	} else if ((card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE) && (stream == 15)) {
		sprintf(str, "Phono/Mic In L");
	} else if ((card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE) && (stream == 16)) {
		sprintf(str, "Phono/Mic In R");
	} else if (stream <= 18) {
		sprintf(str, "H/W In %i", stream - 10);
	} else if (stream == 19) {
		strcpy(str, "S/PDIF In L");
	} else if (stream == 20) {
		strcpy(str, "S/PDIF In R");
	} else {
		strcpy(str, "???");
	}

	frame = gtk_frame_new(str);
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

	vbox = gtk_vbox_new(FALSE, 6);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	hbox = gtk_hbox_new(FALSE, 2);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	adj = gtk_adjustment_new(96, 0, 96, 1, 16, 0);
	mixer_adj[stream-1][0] = adj;
	vscale = gtk_vscale_new(GTK_ADJUSTMENT(adj));
	mixer_vscale[stream-1][0] = vscale;
        gtk_widget_show(vscale);
	gtk_box_pack_start(GTK_BOX(hbox), vscale, TRUE, FALSE, 0);
	gtk_scale_set_value_pos(GTK_SCALE(vscale), GTK_POS_BOTTOM);
	gtk_scale_set_digits(GTK_SCALE(vscale), 0);
	gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
			   GTK_SIGNAL_FUNC(mixer_adjust),
			  (gpointer)((stream << 16) + 0));

	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox1);
	gtk_box_pack_start(GTK_BOX(hbox), vbox1, FALSE, FALSE, 0);

	drawing = gtk_drawing_area_new();
	mixer_drawing[stream-1] = drawing;
	sprintf(drawname, "Mixer%i", stream);
	gtk_widget_set_name(drawing, drawname);
	gtk_widget_show(drawing);
	gtk_signal_connect(GTK_OBJECT(drawing), "expose_event",
			   GTK_SIGNAL_FUNC(level_meters_expose_event), NULL);
	gtk_signal_connect(GTK_OBJECT(drawing), "configure_event",
			   GTK_SIGNAL_FUNC(level_meters_configure_event), NULL);
	gtk_widget_set_events(drawing, GDK_EXPOSURE_MASK);
	gtk_widget_set_usize(drawing, 45, 202);
	gtk_box_pack_start(GTK_BOX(vbox1), drawing, FALSE, FALSE, 1);

	label = gtk_label_new("");
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox1), label, TRUE, TRUE, 2);

	adj = gtk_adjustment_new(96, 0, 96, 1, 16, 0);
	mixer_adj[stream-1][1] = adj;
	vscale = gtk_vscale_new(GTK_ADJUSTMENT(adj));
	mixer_vscale[stream-1][1] = vscale;
        gtk_widget_show(vscale);
	gtk_box_pack_start(GTK_BOX(hbox), vscale, TRUE, FALSE, 0);
	gtk_scale_set_value_pos(GTK_SCALE(vscale), GTK_POS_BOTTOM);
	gtk_scale_set_digits(GTK_SCALE(vscale), 0);
	gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
			   GTK_SIGNAL_FUNC(mixer_adjust),
			  (gpointer)((stream << 16) + 1));
	
	hbox = gtk_hbox_new(TRUE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 0);

	label = gtk_label_new("Left");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	
	label = gtk_label_new("Right");
	gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);

	toggle = gtk_toggle_button_new_with_label("L/R Gang");
	mixer_stereo_toggle[stream-1] = toggle;
	gtk_widget_show(toggle);
	gtk_box_pack_end(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	/* gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), TRUE); */
	
	hbox = gtk_hbox_new(TRUE, 6);
	gtk_widget_show(hbox);
	gtk_box_pack_end(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	toggle = gtk_toggle_button_new_with_label("Mute");
	mixer_mute_toggle[stream-1][0] = toggle;
	gtk_widget_show(toggle);
	gtk_box_pack_start(GTK_BOX(hbox), toggle, FALSE, TRUE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), TRUE);
	gtk_signal_connect(GTK_OBJECT(toggle), "toggled",
			   GTK_SIGNAL_FUNC(mixer_toggled_mute),
			  (gpointer)((stream << 16) + 0));

	toggle = gtk_toggle_button_new_with_label("Mute");
	mixer_mute_toggle[stream-1][1] = toggle;
	gtk_widget_show(toggle);
	gtk_box_pack_start(GTK_BOX(hbox), toggle, FALSE, TRUE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), TRUE);
	gtk_signal_connect(GTK_OBJECT(toggle), "toggled",
			   GTK_SIGNAL_FUNC(mixer_toggled_mute),
			  (gpointer)((stream << 16) + 1));
}

static void create_mixer(GtkWidget *main, GtkWidget *notebook, int page)
{
        GtkWidget *hbox;
        GtkWidget *hbox1;
        GtkWidget *vbox;

	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *drawing;
	GtkWidget *scrolledwindow;
	GtkWidget *viewport;
	int stream;


	hbox = gtk_hbox_new(FALSE, 3);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(notebook), hbox);

        label = gtk_label_new("Monitor Mixer");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), 
				   gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook),page), 
				   label);

	/* create digital mixer frame */
	vbox = gtk_vbox_new(FALSE, 1);
	gtk_widget_show(vbox);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

	frame = gtk_frame_new("Digital Mixer");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 6);


	/* create controls in the digital mixer frame */

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);	

	hbox1 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox1);
	gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 6);

	drawing = gtk_drawing_area_new();
	mixer_mix_drawing = drawing;
	gtk_widget_set_name(drawing, "DigitalMixer");
	gtk_box_pack_start(GTK_BOX(hbox1), drawing, TRUE, FALSE, 6);
	gtk_widget_set_usize(drawing, 98, 276);
	gtk_signal_connect(GTK_OBJECT(drawing), "expose_event",
			   (GtkSignalFunc)level_meters_expose_event, NULL);
	gtk_signal_connect(GTK_OBJECT(drawing), "configure_event",
			   (GtkSignalFunc)level_meters_configure_event, NULL);
	gtk_widget_set_events(drawing, GDK_EXPOSURE_MASK);
	gtk_widget_show(drawing);

	hbox1 = gtk_hbox_new(TRUE, 0);
	gtk_widget_show(hbox1);
	gtk_box_pack_start(GTK_BOX(vbox), hbox1, TRUE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox1), 6);

	label = gtk_label_new("Left");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox1), label, FALSE, TRUE, 0);

	label = gtk_label_new("Right");
	gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox1), label, FALSE, TRUE, 0);


	mixer_clear_peaks_button = gtk_button_new_with_label("Reset Peaks");
	gtk_widget_show(mixer_clear_peaks_button);
	gtk_box_pack_start(GTK_BOX(vbox), mixer_clear_peaks_button, TRUE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(mixer_clear_peaks_button), 6);
	gtk_signal_connect(GTK_OBJECT(mixer_clear_peaks_button), "clicked",
			   GTK_SIGNAL_FUNC(level_meters_reset_peaks), NULL);

	/* build scrolling area */
	scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrolledwindow);
	gtk_box_pack_start(GTK_BOX(hbox), scrolledwindow, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow), 
					GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);

	viewport = gtk_viewport_new(NULL, NULL);
	gtk_widget_show(viewport);
	gtk_container_add(GTK_CONTAINER(scrolledwindow), viewport);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(viewport), vbox);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	for(stream = 1; stream <= output_channels; stream ++)
		create_mixer_frame(hbox, stream);
	for(stream = 11; stream <= input_channels + 10; stream ++)
		create_mixer_frame(hbox, stream);
	for(stream = 19; stream <= spdif_channels + 18; stream ++)
		create_mixer_frame(hbox, stream);
}

static void create_router_frame(GtkWidget *box, int stream, int pos)
{
	GtkWidget *vbox;
	GtkWidget *frame;
	GtkWidget *radiobutton;
	GtkWidget *label;
	GtkWidget *hseparator;
	GSList *group = NULL;
	char str[64], str1[64];
	int idx;
	static char *table[10] = {
		"S/PDIF In L",
		"S/PDIF In R",
		"H/W In 1",
		"H/W In 2",
		"H/W In 3",
		"H/W In 4",
		"H/W In 5",
		"H/W In 6",
		"H/W In 7",
		"H/W In 8"
	};

	if (card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE)
	{
		table[2] = "CD In L";
                table[3] = "CD In R";
                table[4] = "Line In L";
                table[5] = "Line In R";
                table[6] = "Phono/Mic In L";
                table[7] = "Phono/Mic In R";
	}

	if (stream <= 8) {
		sprintf(str, "H/W Out %i (%s)", stream, stream & 1 ? "L" : "R");
	} else if (stream == 9) {
		strcpy(str, "S/PDIF Out (L)");
	} else if (stream == 10) {
		strcpy(str, "S/PDIF Out (R)");
	} else {
		strcpy(str, "???");
	}
	sprintf(str1, "PCM Out %i", stream);

	frame = gtk_frame_new(str);
	gtk_widget_show(frame);
	gtk_box_pack_start (GTK_BOX(box), frame, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 6);


	vbox = gtk_vbox_new(TRUE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);


	radiobutton = gtk_radio_button_new_with_label(group, str1);
	router_radio[stream-1][0] = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)patchbay_toggled, 
			  (gpointer)((stream << 16) + 0));


	hseparator = gtk_hseparator_new();
	gtk_widget_show(hseparator);
	gtk_box_pack_start(GTK_BOX(vbox), hseparator, FALSE, TRUE, 0);

	label = gtk_label_new("");
	gtk_widget_show(label);

	if(stream == 1 || stream == 2 || stream == 9 || stream == 10) {
		radiobutton = gtk_radio_button_new_with_label(group, stream & 1 ? "Digital Mix L" : "Digital Mix R");
		router_radio[stream-1][1] = radiobutton;
		group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
		gtk_widget_show(radiobutton);
		gtk_box_pack_start(GTK_BOX(vbox), 
				    radiobutton, FALSE, FALSE, 0);
		gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
				  (GtkSignalFunc)patchbay_toggled, 
				  (gpointer)((stream << 16) + 1));
	}
	else {
	  label = gtk_label_new("");
	  gtk_widget_show(label);
	  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	}


	hseparator = gtk_hseparator_new();
	gtk_widget_show(hseparator);
	gtk_box_pack_start(GTK_BOX(vbox), hseparator, FALSE, TRUE, 0);


	for(idx = 2 - spdif_channels; idx < input_channels + 2; idx++) {
		radiobutton = gtk_radio_button_new_with_label(group, table[idx]);
		router_radio[stream-1][2+idx] = radiobutton;
		group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
		gtk_widget_show(radiobutton);
		gtk_box_pack_start(GTK_BOX(vbox), 
				    radiobutton, FALSE, FALSE, 0);
		gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
				  (GtkSignalFunc)patchbay_toggled, 
				  (gpointer)((stream << 16) + 2 + idx));
	}
}

static void create_router(GtkWidget *main, GtkWidget *notebook, int page)
{
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *scrolledwindow;
	GtkWidget *viewport;
	int stream, pos;

	scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrolledwindow);
	gtk_container_add(GTK_CONTAINER(notebook), scrolledwindow);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow), 
				       GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);

        label = gtk_label_new("Patchbay / Router");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), 
				   gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), 
				   label);

	viewport = gtk_viewport_new(NULL, NULL);
	gtk_widget_show(viewport);
	gtk_container_add(GTK_CONTAINER(scrolledwindow), viewport);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(viewport), hbox);

	pos = 0;
	for (stream = 1; stream <= output_channels; stream++) {
		if (patchbay_stream_is_active(stream))
			create_router_frame(hbox, stream, pos++);
	}
	for (stream = 8; stream <= 8 + spdif_channels; stream++) {
		if (patchbay_stream_is_active(stream))
			create_router_frame(hbox, stream, pos++);
	}
}

static void create_master_clock(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *radiobutton;
	GtkWidget *label;
	GSList *group = NULL;

	frame = gtk_frame_new("Master Clock");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 0);


	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);


	radiobutton = gtk_radio_button_new_with_label(group, "Int 22050");
	hw_master_clock_xtal_22050 = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)internal_clock_toggled, 
			  (gpointer)"22050");


	radiobutton = gtk_radio_button_new_with_label(group, "Int 32000");
	hw_master_clock_xtal_32000 = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)internal_clock_toggled, 
			  (gpointer)"32000");


	radiobutton = gtk_radio_button_new_with_label(group, "Int 44100");
	hw_master_clock_xtal_44100 = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)internal_clock_toggled, 
			  (gpointer)"44100");


	radiobutton = gtk_radio_button_new_with_label(group, "Int 48000");
	hw_master_clock_xtal_48000 = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)internal_clock_toggled, 
			  (gpointer)"48000");


	radiobutton = gtk_radio_button_new_with_label(group, "Int 88200");
	hw_master_clock_xtal_88200 = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)internal_clock_toggled, 
			  (gpointer)"88200");


	radiobutton = gtk_radio_button_new_with_label(group, "Int 96000");
	hw_master_clock_xtal_96000 = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)internal_clock_toggled, 
			  (gpointer)"96000");



	radiobutton = gtk_radio_button_new_with_label(group, "S/PDIF In");
	hw_master_clock_spdif_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)internal_clock_toggled, 
			  (gpointer)"SPDIF");



	if (card_eeprom.subvendor != ICE1712_SUBDEVICE_DELTA1010)
		return;

	radiobutton = gtk_radio_button_new_with_label(group, "Word Clock");
	hw_master_clock_word_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)internal_clock_toggled, 
			  (gpointer)"WordClock");
	
        label = gtk_label_new("Locked");
        hw_master_clock_status_label = label;
        gtk_widget_show(label);
        gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
}

static void create_rate_state(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *check;

	frame = gtk_frame_new("Rate State");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 0);

	hbox = gtk_hbox_new(TRUE, 0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);

	check = gtk_check_button_new_with_label("Locked");
	hw_rate_locking_check = check;
	gtk_widget_show(check);
	gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(check), "toggled",
			  (GtkSignalFunc)rate_locking_toggled, 
			  (gpointer)"locked");


	check = gtk_check_button_new_with_label("Reset");
	hw_rate_reset_check = check;
	gtk_widget_show(check);
	gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(check), "toggled",
			  (GtkSignalFunc)rate_reset_toggled, 
			  (gpointer)"reset");

}

static void create_actual_rate(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *label;

	frame = gtk_frame_new("Actual Rate");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 0);

	label = gtk_label_new("");
	hw_master_clock_actual_rate_label = label;
	gtk_widget_show(label);
	gtk_container_add(GTK_CONTAINER(frame), label);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_padding(GTK_MISC(label), 6, 6);
}

static void create_volume_change(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkObject *spinbutton_adj;
	GtkWidget *spinbutton;
	GtkWidget *label;

	frame = gtk_frame_new("Volume Change");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);

	label = gtk_label_new("Rate");
	gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 0);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

	spinbutton_adj = gtk_adjustment_new(16, 0, 255, 1, 10, 10);
	hw_volume_change_adj = spinbutton_adj;
	spinbutton = gtk_spin_button_new(GTK_ADJUSTMENT(spinbutton_adj), 1, 0);
	gtk_widget_show(spinbutton);
	gtk_box_pack_start(GTK_BOX(hbox), spinbutton, TRUE, FALSE, 0);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinbutton), TRUE);
	gtk_signal_connect(GTK_OBJECT(spinbutton_adj), "value_changed",
			   GTK_SIGNAL_FUNC(volume_change_rate_adj), NULL);
	
}

static void create_spdif_output_settings_profi_data(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *radiobutton;
	GSList *group = NULL;

	frame = gtk_frame_new("Data Mode");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);


	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);


	radiobutton = gtk_radio_button_new_with_label(group, "Non-audio");
	hw_spdif_profi_nonaudio_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)profi_data_toggled, 
			  (gpointer)"Non-audio");

	radiobutton = gtk_radio_button_new_with_label(group, "Audio");
	hw_spdif_profi_audio_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)profi_data_toggled, 
			  (gpointer)"Audio");
}

static void create_spdif_output_settings_profi_stream(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *radiobutton;
	GSList *group = NULL;

	frame = gtk_frame_new("Stream");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);


	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	radiobutton = gtk_radio_button_new_with_label(group, "Stereophonic");
	hw_profi_stream_stereo_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)profi_stream_toggled, 
			  (gpointer)"Stereo");

	radiobutton = gtk_radio_button_new_with_label(group, "Not indicated");
	hw_profi_stream_notid_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)profi_stream_toggled, 
			  (gpointer)"NOTID");
}

static void create_spdif_output_settings_profi_emphasis(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *radiobutton;
	GSList *group = NULL;

	frame = gtk_frame_new("Emphasis");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);


	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);


	radiobutton = gtk_radio_button_new_with_label(group, "No emphasis");
	hw_profi_emphasis_none_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)profi_emphasis_toggled, 
			  (gpointer)"No");

	radiobutton = gtk_radio_button_new_with_label(group, "50/15us");
	hw_profi_emphasis_5015_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)profi_emphasis_toggled, 
			  (gpointer)"5015");

	radiobutton = gtk_radio_button_new_with_label(group, "CCITT J.17");
	hw_profi_emphasis_ccitt_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)profi_emphasis_toggled, 
			  (gpointer)"CCITT");

	radiobutton = gtk_radio_button_new_with_label(group, "Not indicated");
	hw_profi_emphasis_notid_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)profi_emphasis_toggled, 
			  (gpointer)"NOTID");
}

static void create_spdif_output_settings_profi(GtkWidget *notebook, int page)
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *label;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(notebook), hbox);

        label = gtk_label_new("Professional");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), 
				   gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), 
				   label);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	create_spdif_output_settings_profi_data(vbox);
	create_spdif_output_settings_profi_stream(vbox);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	create_spdif_output_settings_profi_emphasis(vbox);
}

static void create_spdif_output_settings_consumer_copyright(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *radiobutton;
	GSList *group = NULL;

	frame = gtk_frame_new("Copyright");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);


	radiobutton = gtk_radio_button_new_with_label(group, "Copyrighted");
	hw_consumer_copyright_on_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)consumer_copyright_toggled, 
			  (gpointer)"Copyright");

	radiobutton = gtk_radio_button_new_with_label(group, "Copy permitted");
	hw_consumer_copyright_off_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)consumer_copyright_toggled,
			  (gpointer)"Permitted");
}

static void create_spdif_output_settings_consumer_copy(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *radiobutton;
	GSList *group = NULL;

	frame = gtk_frame_new("Copy");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	radiobutton = gtk_radio_button_new_with_label(group,
						      "1-st generation");
	hw_consumer_copy_1st_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)consumer_copy_toggled, 
			  (gpointer)"1st");

	radiobutton = gtk_radio_button_new_with_label(group, "Original");
	hw_consumer_copy_original_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)consumer_copy_toggled, 
			  (gpointer)"Original");
}

static void create_spdif_output_settings_consumer_emphasis(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *radiobutton;
	GSList *group = NULL;

	frame = gtk_frame_new("Emphasis");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	radiobutton = gtk_radio_button_new_with_label(group, "No emphasis");
	hw_consumer_emphasis_none_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)consumer_emphasis_toggled, 
			  (gpointer)"No");

	radiobutton = gtk_radio_button_new_with_label(group, "50/15us");
	hw_consumer_emphasis_5015_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)consumer_emphasis_toggled, 
			  (gpointer)"5015");
}

static void create_spdif_output_settings_consumer_category(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *radiobutton;
	GSList *group = NULL;

	frame = gtk_frame_new("Category");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	radiobutton = gtk_radio_button_new_with_label(group, "DAT");
	hw_consumer_category_dat_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)consumer_category_toggled, 
			  (gpointer)"DAT");

	radiobutton = gtk_radio_button_new_with_label(group, "PCM encoder");
	hw_consumer_category_pcm_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)consumer_category_toggled, 
			  (gpointer)"PCM");

	radiobutton = gtk_radio_button_new_with_label(group, "CD (ICE-908)");
	hw_consumer_category_cd_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)consumer_category_toggled, 
			  (gpointer)"CD");

	radiobutton = gtk_radio_button_new_with_label(group, "General");
	hw_consumer_category_general_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)consumer_category_toggled, 
			  (gpointer)"General");
}

static void create_spdif_output_settings_consumer(GtkWidget *notebook, int page)
{
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(notebook), hbox);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 6);

	label = gtk_label_new("Consumer");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), 
				   gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), 
				   label);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	create_spdif_output_settings_consumer_copyright(vbox);
	create_spdif_output_settings_consumer_copy(vbox);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	create_spdif_output_settings_consumer_emphasis(vbox);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	create_spdif_output_settings_consumer_category(vbox);
}

static void create_spdif_output_settings(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *radiobutton;
	GtkWidget *notebook;
	GSList *group = NULL;

	frame = gtk_frame_new("S/PDIF Output Settings");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 6);


	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	radiobutton = gtk_radio_button_new_with_label(NULL, "Professional");
	hw_spdif_professional_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(hbox), radiobutton, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(radiobutton), 6);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)spdif_output_toggled, 
			  (gpointer)"Professional");

	radiobutton = gtk_radio_button_new_with_label(group, "Consumer");
	hw_spdif_consumer_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(hbox), radiobutton, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(radiobutton), 6);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)spdif_output_toggled, 
			  (gpointer)"Consumer");


	notebook = gtk_notebook_new();
	hw_spdif_output_notebook = notebook;
	gtk_widget_show(notebook);
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);


	create_spdif_output_settings_profi(notebook, 0);
 	create_spdif_output_settings_consumer(notebook, 1); 
}

static void create_spdif_input_select(GtkWidget *box)
{
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *radiobutton;
	GSList *group = NULL;
	int hide = 1;

	if((card_eeprom.subvendor == ICE1712_SUBDEVICE_DELTADIO2496) || (card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE))
		hide = 0;

	frame = gtk_frame_new("S/PDIF Input");
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	radiobutton = gtk_radio_button_new_with_label(group, "Coaxial");
	hw_spdif_input_coaxial_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)spdif_input_toggled, 
			  (gpointer)"Coaxial");

	radiobutton = gtk_radio_button_new_with_label(group, "Optical");
	hw_spdif_input_optical_radio = radiobutton;
	group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
	gtk_widget_show(radiobutton);
	gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
	gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
			  (GtkSignalFunc)spdif_input_toggled, 
			  (gpointer)"Optical");

	if(hide)
		gtk_widget_hide_all(frame);
}

static void create_spdif_on_off(GtkWidget *box)
{
        GtkWidget *frame;
        GtkWidget *vbox;
        GtkWidget *radiobutton;
        GSList *group = NULL;
        int hide = 1;

        if( card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE)
                hide = 0;

        frame = gtk_frame_new("S/PDIF On/Off");
        gtk_widget_show(frame);
        gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

        vbox = gtk_vbox_new(FALSE, 0);
        gtk_widget_show(vbox);
        gtk_container_add(GTK_CONTAINER(frame), vbox);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

        radiobutton = gtk_radio_button_new_with_label(group, "On");
        hw_spdif_switch_on_radio = radiobutton;
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
        gtk_widget_show(radiobutton);
        gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
        gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
                          (GtkSignalFunc)spdif_on_off_toggled,
                          (gpointer)"On");

        radiobutton = gtk_radio_button_new_with_label(group, "Off");
        hw_spdif_switch_off_radio = radiobutton;
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
        gtk_widget_show(radiobutton);
        gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
        gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
                          (GtkSignalFunc)spdif_on_off_toggled,
                          (gpointer)"Off");

        if(hide)
                gtk_widget_hide_all(frame);
}

static void create_breakbox_led(GtkWidget *box)
{
        GtkWidget *frame;
        GtkWidget *vbox;
        GtkWidget *radiobutton;
        GSList *group = NULL;
        int hide = 1;

        if( card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE)
                hide = 0;

        frame = gtk_frame_new("Breakbox LED On/Off");
        gtk_widget_show(frame);
        gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

        vbox = gtk_vbox_new(FALSE, 0);
        gtk_widget_show(vbox);
        gtk_container_add(GTK_CONTAINER(frame), vbox);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

        radiobutton = gtk_radio_button_new_with_label(group, "On");
        hw_breakbox_led_on_radio = radiobutton;
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
        gtk_widget_show(radiobutton);
        gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
        gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
                          (GtkSignalFunc)breakbox_led_toggled,
                          (gpointer)"On");

        radiobutton = gtk_radio_button_new_with_label(group, "Off");
        hw_breakbox_led_off_radio = radiobutton;
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
        gtk_widget_show(radiobutton);
        gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
        gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
                          (GtkSignalFunc)breakbox_led_toggled,
                          (gpointer)"Off");

        if(hide)
                gtk_widget_hide_all(frame);
}

static void create_phono_input(GtkWidget *box)
{
        GtkWidget *frame;
        GtkWidget *vbox;
        GtkWidget *radiobutton;
        GSList *group = NULL;
        int hide = 1;

        if( card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE)
                hide = 0;

        frame = gtk_frame_new("Phono Input Switch");
        gtk_widget_show(frame);
        gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

        vbox = gtk_vbox_new(FALSE, 0);
        gtk_widget_show(vbox);
        gtk_container_add(GTK_CONTAINER(frame), vbox);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

        radiobutton = gtk_radio_button_new_with_label(group, "Phono");
        hw_phono_input_on_radio = radiobutton;
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
        gtk_widget_show(radiobutton);
        gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
        gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
                          (GtkSignalFunc)phono_input_toggled,
                          (gpointer)"Phono");

        radiobutton = gtk_radio_button_new_with_label(group, "Mic");
        hw_phono_input_off_radio = radiobutton;
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
        gtk_widget_show(radiobutton);
        gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
        gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
                          (GtkSignalFunc)phono_input_toggled,
                          (gpointer)"Mic");

        if(hide)
                gtk_widget_hide_all(frame);
}

static void create_input_interface(GtkWidget *box)
{
        GtkWidget *frame;
        GtkWidget *vbox;
        GtkWidget *radiobutton;
        GSList *group = NULL;
        int hide = 1;

        if( card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE)
                hide = 0;

        frame = gtk_frame_new("Line In Selector");
        gtk_widget_show(frame);
        gtk_box_pack_start(GTK_BOX(box), frame, FALSE, TRUE, 0);
        gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

        vbox = gtk_vbox_new(FALSE, 0);
        gtk_widget_show(vbox);
        gtk_container_add(GTK_CONTAINER(frame), vbox);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

        radiobutton = gtk_radio_button_new_with_label(group, "Internal");
        input_interface_internal = radiobutton;
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
        gtk_widget_show(radiobutton);
        gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
        gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
                          (GtkSignalFunc)analog_input_select_toggled,
                          (gpointer)"Internal");

        radiobutton = gtk_radio_button_new_with_label(group, "Front Input");
        input_interface_front_input = radiobutton;
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
        gtk_widget_show(radiobutton);
        gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
        gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
                          (GtkSignalFunc)analog_input_select_toggled,
                          (gpointer)"Front Input");

        radiobutton = gtk_radio_button_new_with_label(group, "Rear Input");
        input_interface_rear_input = radiobutton;
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
        gtk_widget_show(radiobutton);
        gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
        gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
                          (GtkSignalFunc)analog_input_select_toggled,
                          (gpointer)"Rear Input");

        radiobutton = gtk_radio_button_new_with_label(group, "Wave Table");
        input_interface_wavetable = radiobutton;
        group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
        gtk_widget_show(radiobutton);
        gtk_box_pack_start(GTK_BOX(vbox), radiobutton, FALSE, FALSE, 0);
        gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
                          (GtkSignalFunc)analog_input_select_toggled,
                          (gpointer)"Wave Table");

        if(hide)
                gtk_widget_hide_all(frame);
}

static void create_hardware(GtkWidget *main, GtkWidget *notebook, int page)
{
	GtkWidget *label;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *vbox2;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(notebook), hbox);
	label = gtk_label_new("Hardware Settings");
	gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), 
				   gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), 
				   label);


	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

	vbox2 = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox2);
	gtk_box_pack_end(GTK_BOX(hbox),vbox2, FALSE, FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vbox2), 6);

	create_master_clock(vbox);
	create_rate_state(vbox);
	create_actual_rate(vbox);
	create_volume_change(vbox);

 	create_spdif_output_settings(hbox); 

 	create_spdif_input_select(vbox2);
	create_input_interface(vbox2);
	create_breakbox_led(vbox2);
	create_phono_input(vbox2);
	create_spdif_on_off(vbox2);
}

static void create_about(GtkWidget *main, GtkWidget *notebook, int page)
{
	GtkWidget *label;
	GtkWidget *vbox;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(notebook), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

        label = gtk_label_new("About");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), 
				   gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), 
				   label);

	label = gtk_label_new("");
        gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 6);

	/* create first line */
	label = gtk_label_new("Envy24 Control Utility " VERSION);
        gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 6);

	/* create second line */
	label = gtk_label_new("A GTK Tool for Envy24 PCI Audio Chip");
        gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 6);


	/* create third line */
	label = gtk_label_new("Copyright(c) 2000 by Jaroslav Kysela <perex@suse.cz>");
        gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 6);

	label = gtk_label_new("");
        gtk_widget_show(label);
	gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 6);
}

static void create_analog_volume(GtkWidget *main, GtkWidget *notebook, int page)
{
	GtkWidget *label;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *frame;
	GtkObject *adj;
	GtkWidget *vscale;
	GtkWidget *radiobutton;
	GSList *group;
	GtkWidget *scrolledwindow;
	GtkWidget *viewport;
	int i, j;
	static char* dmx6fire_inputs[6] = {
		"CD In (L)",
		"CD In (R)",
		"Line In (L)",
		"Line In (R)",
		"Phono In (L)",
		"Phono In (R)"
	};
	static char* dmx6fire_outputs[6] = {
		"Front (L)",
		"Front (R)",
		"Rear (L)",
		"Rear (R)",
		"Centre",
		"LFE"
	};


	scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrolledwindow);
	gtk_container_add(GTK_CONTAINER(notebook), scrolledwindow);

        label = gtk_label_new("Analog Volume");
        gtk_widget_show(label);
	gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), 
				   gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page), 
				   label);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow), 
				       GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);
	viewport = gtk_viewport_new(NULL, NULL);
	gtk_widget_show(viewport);
	gtk_container_add(GTK_CONTAINER(scrolledwindow), viewport);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(viewport), hbox);

	/* create DAC */
	for(i = 0; i < envy_dac_volumes(); i++) {
		char name[32];
		sprintf(name, "DAC %d", i);
		frame = gtk_frame_new(name);
		gtk_widget_show(frame);
		//gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, TRUE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

		vbox = gtk_vbox_new(FALSE, 0);
		gtk_widget_show(vbox);
		gtk_container_add(GTK_CONTAINER(frame), vbox);
		gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

		/* Add friendly labels for DMX 6Fires */
		if((card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE) && (i < 6)){
			label = gtk_label_new(dmx6fire_outputs[i]);
			gtk_widget_show(label);
			gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 6);
		}

		adj = gtk_adjustment_new(0, -(envy_dac_max()), 0, 1, 16, 0);
		av_dac_volume_adj[i] = adj;
		vscale = gtk_vscale_new(GTK_ADJUSTMENT(adj));
		gtk_scale_set_draw_value(GTK_SCALE(vscale), FALSE);
		gtk_widget_show(vscale);
		gtk_box_pack_start(GTK_BOX(vbox), vscale, TRUE, TRUE, 6);
		gtk_scale_set_digits(GTK_SCALE(vscale), 0);
		gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
				   GTK_SIGNAL_FUNC(dac_volume_adjust), 
				  (gpointer)(i));

	        label = gtk_label_new("000");
	        av_dac_volume_label[i] =(GtkLabel *)label;
	        gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 6);


		if (i >= envy_dac_senses())
			continue;
		group = NULL;
		for (j = 0; j < envy_dac_sense_items(); j++) {
		  radiobutton = gtk_radio_button_new_with_label(group, 
								envy_dac_sense_enum_name(j));
			av_dac_sense_radio[i][j] = radiobutton;
			gtk_widget_show(radiobutton);
			gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
					  (GtkSignalFunc)dac_sense_toggled, 
					  (gpointer)((i << 8) + j));
			gtk_box_pack_start(GTK_BOX(vbox), 
					    radiobutton, FALSE, TRUE, 0);
			group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
		}
	}

	/* create ADC */
	for (i = 0; i < envy_adc_volumes(); i++) {
		char name[32];
		sprintf(name, "ADC %d", i);
		frame = gtk_frame_new(name);
		gtk_widget_show(frame);
		//gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, TRUE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

		vbox = gtk_vbox_new(FALSE, 0);
		gtk_widget_show(vbox);
		gtk_container_add(GTK_CONTAINER(frame), vbox);
		gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

		/* Add friendly labels for DMX 6Fires */
		if((card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE) && (i < 6)){
			label = gtk_label_new(dmx6fire_inputs[i]);
			gtk_widget_show(label);
			gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 6);
		}

		adj = gtk_adjustment_new(0, -127, 0, 1, 16, 0);
		av_adc_volume_adj[i] = adj;
		vscale = gtk_vscale_new(GTK_ADJUSTMENT(adj));
		gtk_scale_set_draw_value(GTK_SCALE(vscale), FALSE);
		gtk_widget_show(vscale);
		gtk_box_pack_start(GTK_BOX(vbox), vscale, TRUE, TRUE, 6);
		gtk_scale_set_value_pos(GTK_SCALE(vscale), GTK_POS_BOTTOM);
		gtk_scale_set_digits(GTK_SCALE(vscale), 0);
		gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
				   GTK_SIGNAL_FUNC(adc_volume_adjust), 
				  (gpointer)(i));

	        label = gtk_label_new("000");
	        av_adc_volume_label[i] =(GtkLabel *)label;
	        gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 6);

		if (i >= envy_adc_senses())
			continue;
		group = NULL;
		for (j = 0; j < envy_adc_sense_items(); j++) {
			radiobutton = gtk_radio_button_new_with_label(group, 
								      envy_adc_sense_enum_name(j));
			av_adc_sense_radio[i][j] = radiobutton;
			gtk_widget_show(radiobutton);
			gtk_signal_connect(GTK_OBJECT(radiobutton), "toggled",
					  (GtkSignalFunc)adc_sense_toggled, 
					  (gpointer)((i << 8) + j));
			gtk_box_pack_start(GTK_BOX(vbox), 
					    radiobutton, FALSE, TRUE, 0);
			group = gtk_radio_button_group(GTK_RADIO_BUTTON(radiobutton));
		}
	}

	/* create IPGA */
	for (i = 0; i < envy_ipga_volumes(); i++) {
		char name[32];
		sprintf(name, "IPGA %d", i);
		frame = gtk_frame_new(name);
		gtk_widget_show(frame);
		//gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, TRUE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(frame), 6);

		vbox = gtk_vbox_new(FALSE, 0);
		gtk_widget_show(vbox);
		gtk_container_add(GTK_CONTAINER(frame), vbox);
		gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

		/* Add friendly labels for DMX 6Fires */
		if((card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE) && (i < 6)){
			label = gtk_label_new(dmx6fire_inputs[i]);
			gtk_widget_show(label);
			gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 6);
		}

		adj = gtk_adjustment_new(0, -36, 0, 1, 16, 0);
		av_ipga_volume_adj[i] = adj;
		vscale = gtk_vscale_new(GTK_ADJUSTMENT(adj));
		gtk_scale_set_draw_value(GTK_SCALE(vscale), FALSE);
		gtk_widget_show(vscale);
		gtk_box_pack_start(GTK_BOX(vbox), vscale, TRUE, TRUE, 6);
		gtk_scale_set_value_pos(GTK_SCALE(vscale), GTK_POS_BOTTOM);
		gtk_scale_set_digits(GTK_SCALE(vscale), 0);
		gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
				   GTK_SIGNAL_FUNC(ipga_volume_adjust), 
				  (gpointer)(i));

	        label = gtk_label_new("000");
	        av_ipga_volume_label[i] = (GtkLabel *)label;
	        gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 6);
	}
}


static void usage(void)
{
	fprintf(stderr, "usage: envy24control [-c card#] [-D control-name] [-o num-outputs] [-i num-inputs]\n");
	fprintf(stderr, "\t-c, --card\tAlsa card number to control\n");
	fprintf(stderr, "\t-D, --device\tcontrol-name\n");
	fprintf(stderr, "\t-o, --outputs\tLimit number of outputs to display\n");
	fprintf(stderr, "\t-i, --input\tLimit number of inputs to display\n");
	fprintf(stderr, "\t-s, --spdif\tLimit number of spdif outputs to display\n");
}

int main(int argc, char **argv)
{
        GtkWidget *notebook;
        char *name, tmpname[8], title[128];
	int i, c, err;
	snd_ctl_card_info_t *hw_info;
	snd_ctl_elem_value_t *val;
	int npfds;
	struct pollfd *pfds;
	int page;
	int output_channels_set = 0;
	int input_channels_set = 0;
	static struct option long_options[] = {
		{"device", 1, 0, 'D'},
		{"card", 1, 0, 'c'},
		{"inputs", 1, 0, 'i'},
		{"outputs", 1, 0, 'o'},
		{"spdif", 1, 0, 's'},
		{0, 0, 0, 0}
	};


	snd_ctl_card_info_alloca(&hw_info);
	snd_ctl_elem_value_alloca(&val);

	/* Go through gtk initialization */
        gtk_init(&argc, &argv);

	name = "hw:0";
	input_channels = 8;
	output_channels = 10;
	spdif_channels = 2;
	while ((c = getopt_long(argc, argv, "D:c:i:o:s:", long_options, NULL)) != -1) {
		switch (c) {
		case 'c':
			i = atoi(optarg);
			if (i < 0 || i >= 8) {
				fprintf(stderr, "envy24control: invalid card number %d\n", i);
				exit(1);
			}
			sprintf(tmpname, "hw:%d", i);
			name = tmpname;
			break;
		case 'D':
			name = optarg;
			break;
		case 'i':
			input_channels = atoi(optarg);
			if (input_channels < 0 || input_channels > 8) {
				fprintf(stderr, "envy24control: must have 0-8 inputs\n");
				exit(1);
			}
			input_channels_set = 1;
			break;
		case 'o':
			output_channels = atoi(optarg);
			if (output_channels < 0 || output_channels > 10) {
				fprintf(stderr, "envy24control: must have 0-10 outputs\n");
				exit(1);
			output_channels_set = 1;
			}
			break;
		case 's':
			spdif_channels = atoi(optarg);
			if (spdif_channels < 0 || spdif_channels > 2) {
				fprintf(stderr, "envy24control: must have 0-2 spdifs\n");
				exit(1);
			}
			break;
		default:
			usage();
			exit(1);
			break;
		}
	}

	if ((err = snd_ctl_open(&ctl, name, 0)) < 0) {
		fprintf(stderr, "snd_ctl_open: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if ((err = snd_ctl_card_info(ctl, hw_info)) < 0) {
		fprintf(stderr, "snd_ctl_card_info: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	if (strcmp(snd_ctl_card_info_get_driver(hw_info), "ICE1712")) {
		fprintf(stderr, "invalid card type (driver is %s)\n", snd_ctl_card_info_get_driver(hw_info));
		exit(EXIT_FAILURE);
	}

	snd_ctl_elem_value_set_interface(val, SND_CTL_ELEM_IFACE_CARD);
	snd_ctl_elem_value_set_name(val, "ICE1712 EEPROM");
	if ((err = snd_ctl_elem_read(ctl, val)) < 0) {
		fprintf(stderr, "Unable to read EEPROM contents: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	memcpy(&card_eeprom, snd_ctl_elem_value_get_bytes(val), 32);

	/* Set a better default for input_channels and output_channels */
	if(!input_channels_set) {
		if(card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE) {
			input_channels = 6;
		}
	}

	if(!output_channels_set) {
		if(card_eeprom.subvendor == ICE1712_SUBDEVICE_DMX6FIRE) {
			output_channels = 6;
		}
	}

	/* Initialize code */
	level_meters_init();
	patchbay_init();
	hardware_init();
	analog_volume_init();

        /* Make the title */
        sprintf(title, "Envy24 Control Utility %s (%s)", VERSION, snd_ctl_card_info_get_longname(hw_info));

        /* Create the main window */
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), title);
        gtk_signal_connect(GTK_OBJECT (window), "delete_event", 
                           (GtkSignalFunc) gtk_main_quit, NULL);
        signal(SIGINT, (void *)gtk_main_quit);

        /* Create the notebook */
        notebook = gtk_notebook_new();
        gtk_widget_show(notebook);
	gtk_container_add(GTK_CONTAINER(window), notebook);

	page = 0;
	create_mixer(window, notebook, page++);

	create_router(window, notebook, page++);
	create_hardware(window, notebook, page++);
	if (envy_analog_volume_available())
		create_analog_volume(window, notebook, page++);
	create_about(window, notebook, page++);

	npfds = snd_ctl_poll_descriptors_count(ctl);
	if (npfds > 0) {
		pfds = alloca(sizeof(*pfds) * npfds);
		npfds = snd_ctl_poll_descriptors(ctl, pfds, npfds);
		for (i = 0; i < npfds; i++)
			gdk_input_add(pfds[i].fd,
				      GDK_INPUT_READ,
				      control_input_callback,
				      ctl);
		snd_ctl_subscribe_events(ctl, 1);
	}
	gtk_timeout_add(40, level_meters_timeout_callback, NULL);
	gtk_timeout_add(100, master_clock_status_timeout_callback, NULL);
	gtk_timeout_add(100, internal_clock_status_timeout_callback, NULL);


	gtk_widget_show(window);

	level_meters_postinit();
	mixer_postinit();
	patchbay_postinit();	
	hardware_postinit();
	analog_volume_postinit();

	gtk_main();

	snd_ctl_close(ctl);

	return EXIT_SUCCESS;
}
