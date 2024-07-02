/**
 * This file is part of Patternshop Project.
 *
 * Patternshop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Patternshop is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Patternshop.  If not, see <http://www.gnu.org/licenses/>
*/
#ifndef PS_MESSAGE_H__

#define PS_MESSAGE_H__

enum	ErrID
{
	ERROR_FILE_ACCESS = 0,
	ERROR_FILE_READ,
	ERROR_FILE_VERSION,
	ERROR_FILE_WRITE,

	ERROR_IMAGE_LOAD,
	ERROR_IMAGE_MATRIX,
	ERROR_IMAGE_SELECT,

	ERROR_MATRIX_SELECT,
	ERROR_MATRIX_NO_EXPORTABLE,

	ERROR_PATTERN_LOAD,
	ERROR_PATTERN_SELECT,

	ERROR_PROJECT_SELECT,

	ERROR_VIDEOCARD,

	ERROR_UNDEF,
	ERROR_NONE

};

enum	QuestID
{
	QUESTION_DELETE_IMAGE = 0,
	QUESTION_DELETE_MATRIX,
	QUESTION_EXPORT_ROTATION,
	QUESTION_LOOSE_ALPHA,
	QUESTION_RESET_CONFIG_FILE,
	QUESTION_SAVE_CLOSING,
	QUESTION_UNDEF
};

enum  LabelID
{
	LABEL_MATRIX = 0,
	LABEL_IMAGE,
	LABEL_PATTERN,
	LABEL_BACKGROUND,
	LABEL_UNDEF
};

enum ActionID
{
	ACTION_DEL_IMAGE = 0,
	ACTION_DEL_MATRIX,
	ACTION_MOVE,
	ACTION_NEW_IMAGE,
	ACTION_NEW_MATRIX,
	ACTION_REPLACE,
	ACTION_RESIZE,
	ACTION_ROTATE,
	ACTION_SWAP_IMAGE,
	ACTION_SWAP_MATRIX,
	ACTION_TORSIO,
	ACTION_PATTERN_ROTATE,
	ACTION_PATTERN_TRANSLATE,
	ACTION_PATTERN_SCALE,
	ACTION_UNDEF
};

bool	GetQuestion(QuestID);
void	GetError(ErrID);
char* GetLabel(LabelID);
char* GetAction(ActionID);

enum PsLanguage
{
	Default,
	English,
	French
};

PsLanguage getLanguage();
void setLanguage(PsLanguage);

#endif /* PS_MESSAGE_H__ */
