*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2003 - 2006, Aheeva Technology.
 *
 * Anatoliy Botov (botov_av@taximaxim.ru)
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 *
 * A license has been granted to Digium (via disclaimer) for the use of
 * this code.
 */

/*! \file
 *
 * \brief Answering machine detection with File playbacks on background
 *
 * \author Anatoliy Botov (botov_av@taximaxim.ru)
 */

/*** MODULEINFO
	<support_level>extended</support_level>
 ***/

#include "asterisk.h"

 ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

 #include "asterisk/module.h"
 #include "asterisk/lock.h"
 #include "asterisk/channel.h"
 #include "asterisk/dsp.h"
 #include "asterisk/pbx.h"
 #include "asterisk/config.h"
 #include "asterisk/app.h"
 #include "asterisk/file.h"

 /*** DOCUMENTATION
 	<application name="AMDFILE" language="en_US">
 		<synopsis>
 			Attempt to detect answering machines with file playbacks on background.
 		</synopsis>
 		<syntax>
 			<parameter name="fileName" required="true">
 				<para>File name that will be playback on background.</para>
 				<para>If file playback is over, recognition process stops.</para>
 			</parameter>
 			<parameter name="initialSilence" required="false">
 				<para>Is maximum initial silence duration before greeting.</para>
 				<para>If this is exceeded, the result is detection as a MACHINE</para>
 			</parameter>
 			<parameter name="greeting" required="false">
 				<para>is the maximum length of a greeting.</para>
 				<para>If this is exceeded, the result is detection as a MACHINE</para>
 			</parameter>
 			<parameter name="afterGreetingSilence" required="false">
 				<para>Is the silence after detecting a greeting.</para>
 				<para>If this is exceeded, the result is detection as a HUMAN</para>
 			</parameter>
 			<parameter name="totalAnalysis Time" required="false">
 				<para>Is the maximum time allowed for the algorithm</para>
 				<para>to decide on whether the audio represents a HUMAN, or a MACHINE</para>
 			</parameter>
 			<parameter name="miniumWordLength" required="false">
 				<para>Is the minimum duration of Voice considered to be a word</para>
 			</parameter>
 			<parameter name="betweenWordSilence" required="false">
 				<para>Is the minimum duration of silence after a word to
 				consider the audio that follows to be a new word</para>
 			</parameter>
 			<parameter name="maximumNumberOfWords" required="false">
 				<para>Is the maximum number of words in a greeting</para>
 				<para>If this is REACHED, then the result is detection as a MACHINE</para>
 			</parameter>
 			<parameter name="silenceThreshold" required="false">
 				<para>What is the average level of noise from 0 to 32767 which if not exceeded, should be considered silence?</para>
 			</parameter>
 			<parameter name="maximumWordLength" required="false">
 				<para>Is the maximum duration of a word to accept.</para>
 				<para>If exceeded, then the result is detection as a MACHINE</para>
 			</parameter>
 		</syntax>
 		<description>
 			<para>This application attempts to detect answering machines at the beginning
 			of outbound calls. Process begins after playback of file starts, and end when playback of file ends.</para>
 			<para>Simply call this application after the call
 			has been answered (outbound only, of course).</para>
 			<para>When loaded, AMD reads amd.conf and uses the parameters specified as
 			default values. Those default values get overwritten when the calling AMD
 			with parameters.</para>
 			<para>This application sets the following channel variables:</para>
 			<variablelist>
 				<variable name="AMDSTATUS">
 					<para>This is the status of the answering machine detection</para>
 					<value name="MACHINE" />
 					<value name="HUMAN" />
 					<value name="NOTSURE" />
 					<value name="HANGUP" />
 				</variable>
 				<variable name="AMDCAUSE">
 					<para>Indicates the cause that led to the conclusion</para>
 					<value name="TOOLONG">
 						Total Time.
 					</value>
 					<value name="INITIALSILENCE">
 						Silence Duration - Initial Silence.
 					</value>
 					<value name="HUMAN">
 						Silence Duration - afterGreetingSilence.
 					</value>
 					<value name="LONGGREETING">
 						Voice Duration - Greeting.
 					</value>
 					<value name="MAXWORDLENGTH">
 						Word Length - max length of a single word.
 					</value>
 					<value name="MAXWORDS">
 						Word Count - maximum number of words.
 					</value>
 				</variable>
 			</variablelist>
 		</description>
 		<see-also>
 			<ref type="application">WaitForSilence</ref>
 			<ref type="application">WaitForNoise</ref>
 		</see-also>
 	</application>

  ***/
