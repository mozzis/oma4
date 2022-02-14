  
#define PRODX
  
{##
   var K, IDENT_TOKLEN: integer;
   begin
   indent:=0;
   ident_toklen:=15;
   writeln('#define IDENT_TOKLEN ',ident_toklen);
   if maxtoklen >ident_toklen then ident_toklen:=maxtoklen;
   writeln('#define MAXTOKLEN ',ident_toklen);
   writeln('#define MAXRPLEN ', maxrplen);
   {length of longest production right member}
   writeln('#define TERM_TOKS ', term_toks);
   { number of terminal tokens }
   writeln('#define NTERM_TOKS ', nterm_toks);
   { number of nonterminal tokens }
   writeln('#define ALL_TOKS  ', term_toks+nterm_toks);
   { all tokens }
   writeln('#define IDENT_TOKX  ', id_tokx);
   { token number of <identifier> }
   writeln('#define INT_TOKX  ', int_tokx);
   { token number of <integer> }
   writeln('#define REAL_TOKX  ', real_tokx);
   { token number of <real> }
   writeln('#define STR_TOKX  ', str_tokx);
   { token number of <string> }
   writeln('#define STOP_TOKX  ', stop_tokx);
   { token number of stopsign (end-of-file) }
   writeln('#define GOAL_TOKX  ', goal_tokx);
   { token number of goal }
   writeln('#define EOL_TOKX  ', eol_tokx);
   { token number of end-of-line }
   writeln('#define REDUCELEN  ', reducestate);
   { last reduce state }
   writeln('#define READSTATE  ', readstate);
   { first READ state }
   writeln('#define LREADSTATE  ', lookstate-1);
   { last READ state }
   writeln('#define LOOKSTATE  ', lookstate);
   { first LOOK state }
   writeln('#define MAXSTATE  ', maxstate);
   { largest state number }
   writeln('#define RLTOKENS  ', udim(toknum));
   { dimension of TOKNUM array }
   writeln('#define SSTOKENS  ', udim(stk_state));
   { dimension of STK_STATE array }
   writeln('#define PRODTOKS  ', udim(prods));
   writeln('#define TOKCHARS  ', term_chars
   +nterm_chars
   +all_toks);
   writeln('#define START_STATE  ', start_state);
   { initial state }
   writeln('#define STK_STATE_1  ', istk_state);
   { state initially pushed on stack }
  
   for k:=ldim(flags) to udim(flags) do
         writeln('#define ', flags[k], ' ', k);
  
         for k:=1 to term_toks do  {wild tokens}
   if is_wild[k] then writeln('#define ', tokens[k], ' ', k);
      end;
   ##}
  
#define ENDPRODX
