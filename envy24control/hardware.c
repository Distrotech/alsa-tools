/*****************************************************************************
   hardware.c - Hardware Settings
   Copyright (C) 2000 by Jaroslav Kysela <perex@suse.cz>
   
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

static snd_ctl_elem_value_t *spdif_master;
static snd_ctl_elem_value_t *word_clock_sync;
static snd_ctl_elem_value_t *volume_rate;
static snd_ctl_elem_value_t *spdif_input;
static snd_ctl_elem_value_t *spdif_output;

#define toggle_set(widget, state) \
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), state);

static int is_active(GtkWidget *widget)
{
	return GTK_TOGGLE_BUTTON(widget)->active ? 1 : 0;
}

void master_clock_update(void)
{
	int err;
	
	if ((err = snd_ctl_elem_read(ctl, spdif_master)) < 0)
		g_print("Unable to read S/PDIF master state: %s\n", snd_strerror(err));
	if (card_eeprom.subvendor == ICE1712_SUBDEVICE_DELTA1010) {
		if ((err = snd_ctl_elem_read(ctl, word_clock_sync)) < 0)
			g_print("Unable to read word clock sync selection: %s\n", snd_strerror(err));
	}
	if (snd_ctl_elem_value_get_boolean(spdif_master, 0)) {
		if (snd_ctl_elem_value_get_boolean(word_clock_sync, 0)) {
			toggle_set(hw_master_clock_word_radio, TRUE);
		} else {
			toggle_set(hw_master_clock_spdif_radio, TRUE);
		}
	} else {
		toggle_set(hw_master_clock_xtal_radio, TRUE);
	}
	master_clock_status_timeout_callback(NULL);
}

static void master_clock_spdif_master(int on)
{
	int err;

	snd_ctl_elem_value_set_boolean(spdif_master, 0, on ? 1 : 0);
	if ((err = snd_ctl_elem_write(ctl, spdif_master)) < 0)
		g_print("Unable to write S/PDIF master state: %s\n", snd_strerror(err));
}

static void master_clock_word_select(int on)
{
	int err;

	if (card_eeprom.subvendor != ICE1712_SUBDEVICE_DELTA1010)
		return;
	snd_ctl_elem_value_set_boolean(word_clock_sync, 0, on ? 1 : 0);
	if ((err = snd_ctl_elem_write(ctl, word_clock_sync)) < 0)
		g_print("Unable to write word clock sync selection: %s\n", snd_strerror(err));
}

void master_clock_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *what = (char *) data;

	if (!is_active(togglebutton))
		return;
	if (!strcmp(what, "Xtal")) {
		master_clock_spdif_master(0);
	} else if (!strcmp(what, "SPDIF")) {
		master_clock_spdif_master(1);
		master_clock_word_select(0);
	} else if (!strcmp(what, "WordClock")) {
		master_clock_spdif_master(1);
		master_clock_word_select(1);
	} else {
		g_print("master_clock_toggled: %s ???\n", what);
	}
}

gint master_clock_status_timeout_callback(gpointer data)
{
	snd_ctl_elem_value_t *sw;
	int err;
	
	if (card_eeprom.subvendor != ICE1712_SUBDEVICE_DELTA1010)
		return FALSE;
	snd_ctl_elem_value_alloca(&sw);
	snd_ctl_elem_value_set_interface(sw, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_value_set_name(sw, "Word Clock Status");
	if ((err = snd_ctl_elem_read(ctl, sw)) < 0)
		g_print("Unable to determine word clock status: %s\n", snd_strerror(err));
	gtk_label_set_text(GTK_LABEL(hw_master_clock_status_label),
			   snd_ctl_elem_value_get_boolean(sw, 0) ? "Locked" : "No signal");
	return TRUE;
}

void volume_change_rate_update(void)
{
	int err;
	
	if ((err = snd_ctl_elem_read(ctl, volume_rate)) < 0)
		g_print("Unable to read volume change rate: %s\n", snd_strerror(err));
	gtk_adjustment_set_value(GTK_ADJUSTMENT(hw_volume_change_adj),
				 snd_ctl_elem_value_get_integer(volume_rate, 0));
}

void volume_change_rate_adj(GtkAdjustment *adj, gpointer data)
{
	int err;
	
	snd_ctl_elem_value_set_integer(volume_rate, 0, adj->value);
	if ((err = snd_ctl_elem_write(ctl, volume_rate)) < 0)
		g_print("Unable to write volume change rate: %s\n", snd_strerror(err));
}

void spdif_output_update(void)
{
	int err;
	snd_aes_iec958_t iec958;
	
	if ((err = snd_ctl_elem_read(ctl, spdif_output)) < 0) {
		if (err == -ENOENT)
			return;
		g_print("Unable to read Delta S/PDIF output state: %s\n", snd_strerror(err));
	}
	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);
	if (!(iec958.status[0] & IEC958_AES0_PROFESSIONAL)) {		/* consumer */
		toggle_set(hw_spdif_consumer_radio, TRUE);
		if (iec958.status[0] & IEC958_AES0_CON_NOT_COPYRIGHT) {
			toggle_set(hw_consumer_copyright_off_radio, TRUE);
		} else {
			toggle_set(hw_consumer_copyright_on_radio, TRUE);
		}
		if ((iec958.status[0] & IEC958_AES0_CON_EMPHASIS) != IEC958_AES0_CON_EMPHASIS_5015) {
			toggle_set(hw_consumer_emphasis_none_radio, TRUE);
		} else {
			toggle_set(hw_consumer_emphasis_5015_radio, TRUE);
		}
		switch (iec958.status[1] & IEC958_AES1_CON_CATEGORY) {
		case IEC958_AES1_CON_MAGNETIC_ID: toggle_set(hw_consumer_category_dat_radio, TRUE); break;
		case IEC958_AES1_CON_DIGDIGCONV_ID: toggle_set(hw_consumer_category_pcm_radio, TRUE); break;
		case IEC958_AES1_CON_GENERAL: toggle_set(hw_consumer_category_general_radio, TRUE); break;
		case IEC958_AES1_CON_LASEROPT_ID:
		default: toggle_set(hw_consumer_category_cd_radio, TRUE); break;
		}
		if (iec958.status[1] & IEC958_AES1_CON_ORIGINAL) {
			toggle_set(hw_consumer_copy_original_radio, TRUE);
		} else {
			toggle_set(hw_consumer_copy_1st_radio, TRUE);
		}
	} else {
		toggle_set(hw_spdif_professional_radio, TRUE);
		if (!(iec958.status[0] & IEC958_AES0_NONAUDIO)) {
			toggle_set(hw_spdif_profi_audio_radio, TRUE);
		} else {
			toggle_set(hw_spdif_profi_nonaudio_radio, TRUE);
		}
		switch (iec958.status[0] & IEC958_AES0_PRO_EMPHASIS) {
		case IEC958_AES0_PRO_EMPHASIS_CCITT: toggle_set(hw_profi_emphasis_ccitt_radio, TRUE); break;
		case IEC958_AES0_PRO_EMPHASIS_NONE: toggle_set(hw_profi_emphasis_none_radio, TRUE); break;
		case IEC958_AES0_PRO_EMPHASIS_5015: toggle_set(hw_profi_emphasis_5015_radio, TRUE); break;
		case IEC958_AES0_PRO_EMPHASIS_NOTID:
		default: toggle_set(hw_profi_emphasis_notid_radio, TRUE); break;
		}
		if ((iec958.status[1] & IEC958_AES1_PRO_MODE) == IEC958_AES1_PRO_MODE_STEREOPHONIC) {
			toggle_set(hw_profi_stream_stereo_radio, TRUE);
		} else {
			toggle_set(hw_profi_stream_notid_radio, TRUE);
		}
	}
}

static void spdif_output_write(void)
{
	int err;

	if ((err = snd_ctl_elem_write(ctl, spdif_output)) < 0)
		g_print("Unable to write Delta S/PDIF Output Defaults: %s\n", snd_strerror(err));
}

void profi_data_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);
	if (!is_active(togglebutton))
		return;
	if (!(iec958.status[0] & IEC958_AES0_PROFESSIONAL))
		return;
	if (!strcmp(str, "Audio")) {
		iec958.status[0] &= ~IEC958_AES0_NONAUDIO;
	} else if (!strcmp(str, "Non-audio")) {
		iec958.status[0] |= IEC958_AES0_NONAUDIO;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void profi_stream_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	if (!is_active(togglebutton))
		return;
	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);
	if (!(iec958.status[0] & IEC958_AES0_PROFESSIONAL))
		return;
	iec958.status[1] &= ~IEC958_AES1_PRO_MODE;
	if (!strcmp(str, "NOTID")) {
		iec958.status[0] |= IEC958_AES1_PRO_MODE_STEREOPHONIC;
	} else if (!strcmp(str, "Stereo")) {
		iec958.status[0] |= IEC958_AES1_PRO_MODE_NOTID;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void profi_emphasis_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);
	if (!is_active(togglebutton))
		return;
	if (!(iec958.status[0] & IEC958_AES0_PROFESSIONAL))
		return;
	iec958.status[0] &= ~IEC958_AES0_PRO_EMPHASIS;
	if (!strcmp(str, "CCITT")) {
		iec958.status[0] |= IEC958_AES0_PRO_EMPHASIS_CCITT;
	} else if (!strcmp(str, "No")) {
		iec958.status[0] |= IEC958_AES0_PRO_EMPHASIS_NONE;
	} else if (!strcmp(str, "5015")) {
		iec958.status[0] |= IEC958_AES0_PRO_EMPHASIS_5015;
	} else if (!strcmp(str, "NOTID")) {
		iec958.status[0] |= IEC958_AES0_PRO_EMPHASIS_NOTID;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void consumer_copyright_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);	
	if (!is_active(togglebutton))
		return;
	if (iec958.status[0] & IEC958_AES0_PROFESSIONAL)
		return;
	if (!strcmp(str, "Copyright")) {
		iec958.status[0] &= ~IEC958_AES0_CON_NOT_COPYRIGHT;
	} else if (!strcmp(str, "Permitted")) {
		iec958.status[1] |= IEC958_AES0_CON_NOT_COPYRIGHT;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void consumer_copy_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);	
	if (!is_active(togglebutton))
		return;
	if (iec958.status[0] & IEC958_AES0_PROFESSIONAL)
		return;
	if (!strcmp(str, "1st")) {
		iec958.status[0] |= IEC958_AES1_CON_ORIGINAL;
	} else if (!strcmp(str, "Original")) {
		iec958.status[1] &= ~IEC958_AES1_CON_ORIGINAL;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void consumer_emphasis_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);	
	if (!is_active(togglebutton))
		return;
	if (iec958.status[0] & IEC958_AES0_PROFESSIONAL)
		return;
	iec958.status[0] &= ~IEC958_AES0_CON_EMPHASIS;
	if (!strcmp(str, "No")) {
		iec958.status[0] |= IEC958_AES0_CON_EMPHASIS_NONE;
	} else if (!strcmp(str, "5015")) {
		iec958.status[1] |= ~IEC958_AES0_CON_EMPHASIS_5015;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void consumer_category_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;

	snd_ctl_elem_value_get_iec958(spdif_output, &iec958);	
	if (!is_active(togglebutton))
		return;
	if (iec958.status[0] & IEC958_AES0_PROFESSIONAL)
		return;
	iec958.status[0] &= ~IEC958_AES1_CON_CATEGORY;
	if (!strcmp(str, "DAT")) {
		iec958.status[0] |= IEC958_AES1_CON_DAT;
	} else if (!strcmp(str, "PCM")) {
		iec958.status[0] |= IEC958_AES1_CON_PCM_CODER;
	} else if (!strcmp(str, "CD")) {
		iec958.status[0] |= IEC958_AES1_CON_IEC908_CD;
	} else if (!strcmp(str, "General")) {
		iec958.status[0] |= IEC958_AES1_CON_GENERAL;
	}
	snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
	spdif_output_write();
}

void spdif_output_toggled(GtkWidget *togglebutton, gpointer data)
{
	char *str = (char *)data;
	snd_aes_iec958_t iec958;
	int page;

	if (is_active(togglebutton)) {
		snd_ctl_elem_value_get_iec958(spdif_output, &iec958);
		if (!strcmp(str, "Professional")) {
			if (!(iec958.status[0] & IEC958_AES0_PROFESSIONAL)) {
				/* default setup: audio, no emphasis */
				memset(&iec958, 0, sizeof(iec958));
				iec958.status[0] = IEC958_AES0_PROFESSIONAL | IEC958_AES0_PRO_EMPHASIS_NONE | IEC958_AES0_PRO_FS_48000;
				iec958.status[1] = IEC958_AES1_PRO_MODE_STEREOPHONIC;
				snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
			}
			page = 0;
		} else {
			if (iec958.status[0] & IEC958_AES0_PROFESSIONAL) {
				/* default setup: no emphasis, PCM encoder */
				memset(&iec958, 0, sizeof(iec958));
				iec958.status[0] = IEC958_AES0_CON_EMPHASIS_NONE;
				iec958.status[1] = IEC958_AES1_CON_PCM_CODER | IEC958_AES1_CON_ORIGINAL;
				iec958.status[3] = IEC958_AES3_CON_FS_48000;
				snd_ctl_elem_value_set_iec958(spdif_output, &iec958);
			}
			page = 1;
		}
		spdif_output_write();
		gtk_notebook_set_page(GTK_NOTEBOOK(hw_spdif_output_notebook), page);
		spdif_output_update();
	}
}

void spdif_input_update(void)
{
	int err;
	
	if (card_eeprom.subvendor != ICE1712_SUBDEVICE_DELTADIO2496)
		return;
	if ((err = snd_ctl_elem_read(ctl, spdif_input)) < 0)
		g_print("Unable to read S/PDIF input switch: %s\n", snd_strerror(err));
	if (snd_ctl_elem_value_get_boolean(spdif_input, 0)) {
		toggle_set(hw_spdif_input_optical_radio, TRUE);
	} else {
		toggle_set(hw_spdif_input_coaxial_radio, TRUE);
	}
}

void spdif_input_toggled(GtkWidget *togglebutton, gpointer data)
{
	int err;
	char *str = (char *)data;
	
	if (!is_active(togglebutton))
		return;
	if (!strcmp(str, "Optical"))
		snd_ctl_elem_value_set_boolean(spdif_input, 0, 1);
	else
		snd_ctl_elem_value_set_boolean(spdif_input, 0, 0);
	if ((err = snd_ctl_elem_write(ctl, spdif_input)) < 0)
		g_print("Unable to write S/PDIF input switch: %s\n", snd_strerror(err));
}

void hardware_init(void)
{
	if (snd_ctl_elem_value_malloc(&spdif_master) < 0 ||
	    snd_ctl_elem_value_malloc(&word_clock_sync) < 0 ||
	    snd_ctl_elem_value_malloc(&volume_rate) < 0 ||
	    snd_ctl_elem_value_malloc(&spdif_input) < 0 ||
	    snd_ctl_elem_value_malloc(&spdif_output) < 0) {
		g_print("Cannot allocate memory\n");
		exit(1);
	}

	snd_ctl_elem_value_set_interface(spdif_master, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_value_set_name(spdif_master, "Multi Track IEC958 Master");

	snd_ctl_elem_value_set_interface(word_clock_sync, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_value_set_name(word_clock_sync, "Word Clock Sync");

	snd_ctl_elem_value_set_interface(volume_rate, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_value_set_name(volume_rate, "Multi Track Volume Rate");

	snd_ctl_elem_value_set_interface(spdif_input, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_value_set_name(spdif_input, "IEC958 Input Optical");

	snd_ctl_elem_value_set_interface(spdif_output, SND_CTL_ELEM_IFACE_PCM);
	snd_ctl_elem_value_set_name(spdif_output, "IEC958 Playback Default");
}

void hardware_postinit(void)
{
	master_clock_update();
	volume_change_rate_update();
	spdif_input_update();
	spdif_output_update();
}
