#ifndef	SHELL
#define	SHELL
#endif

// region 0
  /* cursor move command */
SHELL	void	op_cursor_up();
SHELL	void	op_cursor_down();
SHELL	void	op_cursor_left();
SHELL	void	op_cursor_right();
SHELL	void	op_cursor_tkprev();
SHELL	void	op_cursor_tknext();
SHELL	void	op_cursor_sleft();
SHELL	void	op_cursor_sright();
SHELL	void	op_cursor_sup();
SHELL	void	op_cursor_sdown();
SHELL	void	op_cursor_top();
SHELL	void	op_cursor_bottom();
SHELL	void	op_cursor_pup();
SHELL	void	op_cursor_pdown();
SHELL	void	op_cursor_rup();
SHELL	void	op_cursor_rdown();

  /* line jump command */
SHELL	void	op_jump_before();
SHELL	void	op_jump_mark();
SHELL	void	op_jump_line();
SHELL	void	op_jump_tag();

  /* edit command */
SHELL	void	op_char_input();
SHELL	void	op_char_undo();
SHELL	void	op_line_cr();
SHELL	void	op_line_new();
SHELL	void	op_line_undo();

  /* delete command */
SHELL	void	op_del_bs();
SHELL	void	op_del_char();
SHELL	void	op_del_tkprev();
SHELL	void	op_del_tknext();
SHELL	void	op_del_sleft();
SHELL	void	op_del_sright();


  /* misc command */
SHELL	void	op_opt_set();
SHELL	void	op_opt_linenum();
SHELL	void	op_opt_kanji();
SHELL	void	op_opt_retmode();
SHELL	void	op_opt_tab();

SHELL	void	op_misc_kmacro();
SHELL	void	op_misc_exec();
SHELL	void	op_misc_redraw();
SHELL	void	op_misc_insert_output( ) ;	/* exec and insert stdout/stderr */
SHELL	void	op_menu_opt();
SHELL	void	op_menu_file();


  /* block command */
SHELL	void	op_block_start();
SHELL	void	op_block_cut();
SHELL	void	op_block_paste();
SHELL	void	op_block_yanc();
SHELL	void	op_block_copy();
SHELL	void	op_block_kill();
SHELL	void	op_block_dup();
SHELL	void	op_block_chlast();



  /* search/replace command */
SHELL	void	op_search_in();
SHELL	void	op_search_repl();
SHELL	void	op_search_repl_redo();
SHELL	void	op_search_prev();
SHELL	void	op_search_next();
SHELL	void	op_search_getword();
SHELL	void	op_search_paging();


  /* file command */
SHELL	void	op_file_toggle();
SHELL	void	op_file_select();
SHELL	void	op_file_open();
SHELL	void	op_file_insert();
SHELL	void	op_file_close();
SHELL	void	op_file_save();
SHELL	void	op_file_aclose();
SHELL	void	op_file_copen();
SHELL	void	op_file_undo();
SHELL	void	op_file_rename();
SHELL	void	op_file_quit();

SHELL	void	op_file_split();
SHELL	void	op_file_splitmove();
SHELL	void	op_file_readonly();
SHELL	void	op_file_duplicate();
SHELL	void	op_file_open_new();
SHELL	void	op_file_open_readonly();


// region 1

// SHELL	op_SysScrolldown
// SHELL	op_SysScrollup
// SHELL	op_SysRollup
// SHELL	op_SysRolldown
// SHELL	op_SysReturn
// SHELL	op_SysBackspace
// SHELL	op_SysDeletechar
// SHELL	op_SysCntrlInput
// SHELL	op_SysCursorupside
// SHELL	op_SysCursordownside
// SHELL	op_SysCursorleftside
// SHELL	op_SysCursorrightside
// SHELL	op_SysCursortopside
// SHELL	op_SysCursorendside
// SHELL	op_SysCursorup
// SHELL	op_SysCursordown
// SHELL	op_SysCursorleft
// SHELL	op_SysCursorright
// SHELL	op_SysEscape

// region 2
// SHELL	op_EffCursorUp
// SHELL	op_EffCursorDown
// SHELL	op_EffPageUp
// SHELL	op_EffPageDown
// SHELL	op_EffRollUp
// SHELL	op_EffRollDown

// SHELL	op_EffWindowChange
// SHELL	op_EffWindowNumChange
// SHELL	op_EffReRead
// SHELL	op_EffRename
// SHELL	op_EffMkdir

// SHELL	op_EffExec

// SHELL	op_EffMarkChange
// SHELL	op_EffMarkChangeAll
// SHELL	op_EffFileCp
// SHELL	op_EffFileMv
// SHELL	op_EffFileRm

// SHELL	op_EffBackSpace
// SHELL	op_EffChangeDir
// SHELL	op_EffRootDir
// SHELL	op_EffHomeDir
// SHELL	op_EffReturn
// SHELL	op_EffEscape

