#ifndef HLP_CSR_DSP_8350_H__
#define HLP_CSR_DSP_8350_H__
#include "sysdep.h"
#include "a2dp_ui.h"

/* DSP common interfaces.*/
void csr_dsp_8350_init();
void csr_dsp_8350_on_hardware_reset();
void csr_dsp_8350_on_gap_started();
void csr_dsp_8350_on_vendor_event_cbk(void *par, UINT32 size);

/* Handsfree DSP interfaces */
void csr_dsp_8350_start_ringtone();
void csr_dsp_8350_stop_ringtone();
void csr_dsp_8350_close_ringtone();
void csr_dsp_8350_on_sco_established(UINT16 sco_handle);
void csr_dsp_8350_on_sco_released();
void csr_dsp_8350_set_codec_type(UINT8 codec);

/* A2DP DSP interfaces */
void csr_dsp_8350_on_stream_open(UINT16 cid, UINT16 conn_hdl);
void csr_dsp_8350_on_stream_start();
void csr_dsp_8350_on_stream_suspend();
void csr_dsp_8350_on_stream_done();
int csr_dsp_8350_on_stream_set_config(UINT8 codec, UINT16 sample);
int hlp_a2dp_get_cid(void);

#endif /*HLP_CSR_DSP_8350_H__*/

