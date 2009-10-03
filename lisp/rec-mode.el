;;; rec-mode.el --- Major mode for viewing/editing rec files

;; Copyright (C) 2009 Jose E. Marchesi

;; Maintainer: Jose E. Marchesi

;; This file is NOT part of GNU Emacs.

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program; see the file COPYING.  If not, write to the
;; Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
;; Boston, MA 02110-1301, USA.

;;; Commentary:

;; A major mode for editing rec files.

;;; Code:

(defgroup rec-mode nil
  "rec-mode subsystem"
  :group 'applications
  :link '(url-link "http://www.gnu.org/software/rec"))

(defvar rec-mode-syntax-table
  (let ((st (make-syntax-table)))
    (modify-syntax-entry ?# "<" st)   ; Comment start
    (modify-syntax-entry ?\n ">" st)  ; Comment end
    st)
  "Syntax table used in rec-mode")

(defvar rec-font-lock-keywords
  '(("^%\\(rec\\|key\\|unique\\|mandatory\\):" . font-lock-keyword-face)
    ("^[^:]+:" . font-lock-variable-name-face))
  "Font lock keywords used in rec-mode")
  
(defvar rec-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map (kbd "TAB") 'rec-goto-next-rec)
    map)
  "Keymap for rec-mode")

(defvar rec-comment-re "^#.*$"
  "regexp denoting a comment line")

(setq rec-field-name-re "^[a-zA-Z0-1_%-]+:")
(setq rec-field-re
      (concat rec-field-name-re ;; The field name
              "[ \t]*"          ;; Stripped prefix of blanks
              ".*"              ;; The field value
              "[ \t]*"          ;; Stripped end of blanks
              "\n"))

(defun rec-get-field-value ()
  "Return the field value under the pointer.  If the pointer is not
in top of a field value, return nil."
  (save-excursion
    (let (ret)
      ;; Search the field name
      (when (re-search-backward rec-field-name-re nil t)
        (goto-char (match-end 0))
        ;; Get the field value
        (setq ret (rec-parse-field-value))))))

(defun rec-parse-field-name ()
  "Parse and return a field name starting at point.  If the point
is not at the beginning of a field name return nil."
  (when (and (equal (current-column) 0)
             (looking-at "^[a-zA-Z_%-]+:"))
    (goto-char (match-end 0))
    ;; TODO: strip blank suffix
    (buffer-substring-no-properties (match-beginning 0)
                                    (- (match-end 0) 1))))

(defun rec-parse-field-value ()
  "Parse and return the field value starting at point."
  (save-excursion
    ;; Skip whitespaces
    (re-search-forward "[ \t]+" nil t)
    ;; Get the field value
    (let (exit (val ""))
      (while (and (looking-at "[^\n]+")
                  (or (equal (char-before (match-end 0)) ?\\)
                      (equal (char-after (+ (match-end 0) 1)) ?+)))
        (setq val (concat val (buffer-substring-no-properties (match-beginning 0)
                                                              (match-end 0))))
        (goto-char (match-end 0)))
      val)))
          
(defun rec-parse-field ()
  "Return a structure describing the field starting from the
pointer.  If the pointer is not in the beginning of a field
descriptor then return nil.

The structure containing the data of the field is a list whose
first element is the name of the field and the second element is
the value of the field."
  (let (field-name field-value)
    (and (setq field-name (rec-parse-field-name))
         (setq field-value (rec-parse-field-value)))
    (when (and field-name field-value)
        (list field-name field-value))))
        
(defun rec-goto-next-rec ()
  "Move the pointer to the beginning of the next record."
  (interactive)
  (if (re-search-forward rec-field-re nil t)
      (goto-char (match-beginning 0))))

(defun rec-mode ()
  "A major mode for editing rec files.

Commands:
\\{rec-mode-map}"
  (interactive)
  (kill-all-local-variables)
  (make-local-variable 'font-lock-defaults)
  (setq font-lock-defaults '(rec-font-lock-keywords))
  (use-local-map rec-mode-map)
  (set-syntax-table rec-mode-syntax-table)
  (setq mode-name "Rec")
  (setq major-mode 'rec-mode))

;;; rec-mode.el ends here
