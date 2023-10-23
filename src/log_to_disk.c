void log_to_disk(int from_lan) {
    FILE *mobile_log_file = fopen("mobile_log.adi", "a"); // Open for appending
    if (mobile_log_file == NULL) {
        // Handle file open error
        return;
    }

    pthread_mutex_lock(&disk_mutex);

    if (!from_lan) { // QSO from this node
        // Remember call and report for resend after QSO
        strcpy(lastcall, current_qso.call);

        // Use normalized comment if available
        if (strlen(current_qso.normalized_comment) > 0) {
            strcpy(current_qso.comment, current_qso.normalized_comment);
        }

        restart_band_timer();

        struct qso_t *qso = collect_qso_data();
        addcall(qso); // Add call to dupe list

       // Log the QSO to the "mobile_log" file in ADIF format
       fprintf(mobile_log_file, "<CALL:%d>%s\n", strlen(current_qso.call), current_qso.call);
       fprintf(mobile_log_file, "<BAND:%d>%s\n", strlen(current_qso.band), current_qso.band);
       fprintf(mobile_log_file, "<MODE:%d>%s\n", strlen(current_qso.mode), current_qso.mode);
       fprintf(mobile_log_file, "<FREQ:%d>%s\n", strlen(current_qso.freq), current_qso.freq);
       fprintf(mobile_log_file, "<OPERATOR:%d>%s\n", strlen(current_qso.operator), current_qso.operator);
       fprintf(mobile_log_file, "<TIME_ON:%d>%s\n", strlen(current_qso.time_on), current_qso.time_on);

       // Add the grid square to the mobile log
       fprintf(mobile_log_file, "<GRIDSQUARE:%d>%s\n", strlen(grid_square), grid_square);


        // Add more fields as needed
        // For example, if you have a frequency field, add it here:
        // fprintf(mobile_log_file, "<FREQ:%d>%s\n", strlen(current_qso.freq), current_qso.freq);

        score_qso(qso);
        char *logline = makelogline(qso);
        qso->logline = logline; /* Remember formatted line in qso entry */

        store_qso(logfile, logline);
        g_ptr_array_add(qso_array, qso);

        // Send QSO to other nodes...
        send_lan_message(LOGENTRY, logline);

        if (trx_control && (cqmode == S_P)) {
            add_local_spot();
        }

        cleanup_qso(); /* Reset QSO-related parameters */
    } else { // QSO from LAN
        g_strlcpy(lan_logline, lan_message + 2, 81);
        char *fill = g_strnfill(80 - strlen(lan_logline), ' ');
        g_strlcat(lan_logline, fill, 81); /* Fill with spaces if needed */

        if (cqwwm2) { /* Mark as coming from other station */
            if (lan_message[0] != thisnode) {
                lan_logline[79] = '*';
            }
        }

        total = total + score2(lan_logline);

        struct qso_t *qso = parse_qso(lan_logline);
        addcall2();

        // Log the QSO to the "mobile_log" file in ADIF format
        fprintf(mobile_log_file, "<CALL:%d>%s\n", strlen(current_qso.call), current_qso.call);
        fprintf(mobile_log_file, "<BAND:%d>%s\n", strlen(current_qso.band), current_qso.band);

        // Add the grid square to the mobile log
        fprintf(mobile_log_file, "<GRIDSQUARE:%d>%s\n", strlen(grid_square), grid_square);

        // Add more fields as needed

        store_qso(logfile, lan_logline);
        g_ptr_array_add(qso_array, qso);
    }

    if (from_lan) {
        lan_mutex = 2;
    } else {
        lan_mutex = 1;
    }

    scroll_log();
    lan_mutex = 0;

    attron(modify_attr(COLOR_PAIR(NORMCOLOR)); /* Erase comment field */

    if (!from_lan) {
        mvaddstr(12, 54, spaces(contest->exchange_width));
    }

    attron(COLOR_PAIR(C_LOG) | A_STANDOUT);
    if (!from_lan) {
        mvaddstr(7, 0, logline0);
        mvaddstr(8, 0, logline1);
        mvaddstr(9, 0, logline2);
    }
    mvaddstr(10, 0, logline3);
    mvaddstr(11, 0, logline4);
    refreshp();

    attron(COLOR_PAIR(C_WINDOW));

    mvaddstr(12, 23, qsonrstr);

    if (no_rst) {
        mvaddstr(12, 44, "   ");
        mvaddstr(12, 49, "   ");
    } else {
        mvaddstr(12, 44, sent_rst);
        mvaddstr(12, 49, recvd_rst);
    }

    sync();

    if (rit) {
        set_outfreq(RESETRIT);
    }

    block_part = 0; /* Unblock use partials */

    pthread_mutex_unlock(&disk_mutex);

    // Close the "mobile_log" file at the end of the function.
    fclose(mobile_log_file);
}
