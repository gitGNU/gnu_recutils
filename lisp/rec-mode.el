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

(defun rec-mode ()
  "A major mode for editing rec files.

Commands:
\\{rec-mode-map}"
  (interactive)
  (kill-all-local-variables)
  (setq rec-mode-map (make-keymap))
  (setq mode-name "Rec")
  (setq major-mode 'rec-mode))

;;; rec-mode.el ends here
