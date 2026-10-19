#ifndef _BT_PCM_H_
#define _BT_PCM_H_

//mic 
void open_pcm_write(unsigned int card, unsigned int device, unsigned int channels,
					unsigned int rate, unsigned int bits, unsigned int period_size,
					unsigned int period_count);

void close_pcm_write();
void write_pcm_data(unsigned char *record_buf,int count);


//speaker
//start record pcm thread
void open_pcm_capture(unsigned int card, unsigned int device, unsigned int channels,
					unsigned int rate, unsigned int bits, unsigned int period_size,
					unsigned int period_count);
void close_pcm_capture();

#endif
