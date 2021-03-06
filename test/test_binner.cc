#define GTEST_HAS_TR1_TUPLE 0
#include "../src/binner.h"
#include "../src/parse_buffer.h"
#include "../src/token.h"
#include <gmock/gmock.h>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <memory>

using namespace ::testing;

class test_binner : public ::testing::Test {};

TEST( test_binner, test_binner_keys_single_word_entry_correctly ) {
    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();
    std::unique_ptr<std::vector<TokenWordPair>> line = std::make_unique<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );

    std::unique_ptr<std::vector<TokenWordPair>> line2 = std::make_unique<std::vector<TokenWordPair>>( *line );
    buffer->addLine( std::move( line ) );

    binner.binEntriesInBuffer( std::move( buffer ) );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    ASSERT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( *(vec.at(0).getLine()), *line2 );

    //The lines will be destroyed when the binner shuts down
}

TEST( test_binner, test_binner_keys_different_word_size_different_buckets ) {
    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();

    //One word
    std::unique_ptr<std::vector<TokenWordPair>> line = std::make_unique<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );

    std::unique_ptr<std::vector<TokenWordPair>> loc_line1 = std::make_unique<std::vector<TokenWordPair>>( *line );
    buffer->addLine( std::move( line ) );

    //Two words
    std::unique_ptr<std::vector<TokenWordPair>> line2 = std::make_unique<std::vector<TokenWordPair>>();
    twp.tok = WORD;
    twp.word = "TEST";
    line2->push_back( twp );
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );
    std::unique_ptr<std::vector<TokenWordPair>> loc_line2 = std::make_unique<std::vector<TokenWordPair>>( *line2 );
    buffer->addLine( std::move( line2 ) );

    binner.binEntriesInBuffer( std::move( buffer ) );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 2 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( *(vec.at(0).getLine()), *loc_line1 );

    bk.num_words = 2;
    search = map.find( bk );
    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec2 = search->second.getBinVector();
    EXPECT_EQ( vec2.size(), 1 );
    EXPECT_EQ( *(vec2.at(0).getLine()), *loc_line2 );
}

TEST( test_binner, same_entry_word_match_removed ) {
    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();

    //One word
    std::unique_ptr<std::vector<TokenWordPair>> line = std::make_unique<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );
    std::unique_ptr<std::vector<TokenWordPair>> loc_line1 = std::make_unique<std::vector<TokenWordPair>>( *line );
    buffer->addLine( std::move( line ) );

    //Different line, but same content
    std::unique_ptr<std::vector<TokenWordPair>> line2 = std::make_unique<std::vector<TokenWordPair>>();
    twp.tok = WORD;
    twp.word = "TEST";
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );
    buffer->addLine( std::move( line2 ) );

    binner.binEntriesInBuffer( std::move( buffer ) );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 1 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( *(vec.at(0).getLine()), *loc_line1 );
}

TEST( test_binner, same_entry_word_mismatch_kept ) {
    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();

    //One word
    std::unique_ptr<std::vector<TokenWordPair>> line = std::make_unique<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );
    std::unique_ptr<std::vector<TokenWordPair>> loc_line1 = std::make_unique<std::vector<TokenWordPair>>( *line );
    buffer->addLine( std::move( line ) );

    //Different line, but same content
    std::unique_ptr<std::vector<TokenWordPair>> line2 = std::make_unique<std::vector<TokenWordPair>>();
    twp.tok = WORD;
    twp.word = "TEST2";
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );
    std::unique_ptr<std::vector<TokenWordPair>> loc_line2 = std::make_unique<std::vector<TokenWordPair>>( *line2 );
    buffer->addLine( std::move( line2 ) );

    binner.binEntriesInBuffer( std::move( buffer ) );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 1 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 2 );
    EXPECT_EQ( *(vec.at(0).getLine()), *loc_line1 );
    EXPECT_EQ( *(vec.at(1).getLine()), *loc_line2 );

}

TEST( test_binner, same_word_different_params ) {
    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();

    //One word
    std::unique_ptr<std::vector<TokenWordPair>> line = std::make_unique<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );
    std::unique_ptr<std::vector<TokenWordPair>> loc_line1 = std::make_unique<std::vector<TokenWordPair>>( *line );
    buffer->addLine( std::move( line ) );

    //Different line, but same content
    std::unique_ptr<std::vector<TokenWordPair>> line2 = std::make_unique<std::vector<TokenWordPair>>();
    twp.tok = WORD;
    twp.word = "TEST";
    line2->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );
    std::unique_ptr<std::vector<TokenWordPair>> loc_line2 = std::make_unique<std::vector<TokenWordPair>>( *line2 );
    buffer->addLine( std::move( line2 ) );

    binner.binEntriesInBuffer( std::move( buffer ) );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 2 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 0;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( *(vec.at(0).getLine()), *loc_line1 );

    bk.num_params = 1;
    search = map.find( bk );
    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec2 = search->second.getBinVector();
    EXPECT_EQ( vec2.size(), 1 );
    EXPECT_EQ( *(vec2.at(0).getLine()), *loc_line2 );

}

TEST( test_binner, no_match_abstracted_vals ) {
    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();

    //One word
    std::unique_ptr<std::vector<TokenWordPair>> line = std::make_unique<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "BLAH"; //doesn't match
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );
    std::unique_ptr<std::vector<TokenWordPair>> loc_line1 = std::make_unique<std::vector<TokenWordPair>>( *line );
    buffer->addLine( std::move( line ) );

    //Different line, but same content
    std::unique_ptr<std::vector<TokenWordPair>> line2 = std::make_unique<std::vector<TokenWordPair>>();
    twp.tok = WORD;
    twp.word = "TEST";
    line2->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );
    buffer->addLine( std::move( line2 ) );

    binner.binEntriesInBuffer( std::move( buffer ) );
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    EXPECT_EQ( map.size(), 1 );

    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 1;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );
    EXPECT_EQ( *(vec.at(0).getLine()), *loc_line1 );
}

TEST( test_binner, test_serialize ) {
    std::ofstream os;
    unlink( "test_serialize.txt" );
    os.open( "test_serialize.txt", std::ofstream::out );

    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();

    //One word
    std::unique_ptr<std::vector<TokenWordPair>> line = std::make_unique<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "BLAH"; //doesn't match
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );
    buffer->addLine( std::move( line ) );

    //Different line, but same content
    std::unique_ptr<std::vector<TokenWordPair>> line2 = std::make_unique<std::vector<TokenWordPair>>();
    twp.tok = WORD;
    twp.word = "TEST";
    line2->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );
    buffer->addLine( std::move( line2 ) );

    binner.binEntriesInBuffer( std::move( buffer ) );
    binner.serialize( os );
    os.close();

    std::ifstream is;
    is.open( "test_serialize.txt", std::ifstream::in );

    Binner binner2( &pbe_in );
    std::cout << "Going to deserialize" << std::endl;
    binner2.deserialize( is );
    std::cout << "Done deserializing..." << std::endl;

    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner2.getUnderlyingMap();
    std::unordered_map<BinKey, Bin, BinKeyHasher> &map2 = binner.getUnderlyingMap();
    EXPECT_EQ( map.size(), map2.size() );
    BinKey bk;
    bk.num_words = 1;
    bk.num_params = 1;
    auto search = map.find( bk );
    EXPECT_NE( search, map.end() );
    auto search2 = map2.find( bk );
    EXPECT_NE( search2, map.end() );

    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    std::vector<LineWithTransitions> &vec2 = search2->second.getBinVector();
    EXPECT_EQ( vec.size(), vec2.size() );
    EXPECT_EQ( vec.size(), 1 );
    const std::shared_ptr<std::vector<TokenWordPair>> &recon_line1 = vec.at(0).getLine();
    const std::shared_ptr<std::vector<TokenWordPair>> &recon_line2 = vec2.at(0).getLine();
    EXPECT_NE( &recon_line1, &recon_line2 );

    EXPECT_EQ( recon_line1->at(0).tok, recon_line2->at(0).tok );
    EXPECT_STREQ( recon_line1->at(0).word.c_str(), recon_line2->at(0).word.c_str() );
    EXPECT_EQ( recon_line1->at(1).tok, recon_line2->at(1).tok );
    EXPECT_STREQ( recon_line1->at(1).word.c_str(), recon_line2->at(1).word.c_str() );
    EXPECT_EQ( recon_line1->at(2).tok, recon_line2->at(2).tok );
    EXPECT_STREQ( recon_line1->at(2).word.c_str(), recon_line2->at(2).word.c_str() );
}

TEST( test_transition_counter, create_simple_counter ) {
    std::shared_ptr<std::vector<TokenWordPair>> line = std::make_shared<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );

    LineWithTransitions lwt( line );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 0.0 );
    lwt.addTransition( line );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 1.0 );
}

TEST( test_transition_counter, multiple_transitions ) {
    std::shared_ptr<std::vector<TokenWordPair>> line = std::make_shared<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = WORD;
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );

    std::shared_ptr<std::vector<TokenWordPair>> line2 = std::make_shared<std::vector<TokenWordPair>>();
    twp.tok = WORD;
    twp.word = "TEST";
    line2->push_back( twp );
    twp.tok = WORD;
    twp.word = "BLAH"; //doesn't match
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );

    LineWithTransitions lwt( line );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 0.0 );
    lwt.addTransition( line );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 1.0 );
    lwt.addTransition( line2 );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 0.5 );
    EXPECT_EQ( lwt.getTransitionProbability( line2 ), 0.5 );
}

TEST( test_transition_counter, multiple_lines_and_transitions ) {
    std::shared_ptr<std::vector<TokenWordPair>> line = std::make_shared<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "TEST";
    line->push_back( twp );
    twp.tok = WORD;
    line->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line->push_back( twp );

    std::shared_ptr<std::vector<TokenWordPair>> line2 = std::make_shared<std::vector<TokenWordPair>>();
    twp.tok = WORD;
    twp.word = "TEST";
    line2->push_back( twp );
    twp.tok = WORD;
    twp.word = "BLAH"; //doesn't match
    line2->push_back( twp );
    twp.tok = NEW_LINE;
    twp.word = "\n";
    line2->push_back( twp );

    LineWithTransitions lwt( line );
    LineWithTransitions lwt2( line2 );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 0.0 );
    lwt.addTransition( line );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 1.0 );
    lwt.addTransition( line2 );
    EXPECT_EQ( lwt.getTransitionProbability( line ), 0.5 );
    EXPECT_EQ( lwt.getTransitionProbability( line2 ), 0.5 );

    EXPECT_EQ( lwt2.getTransitionProbability( line ), 0.0 );
    lwt2.addTransition( line );
    EXPECT_EQ( lwt2.getTransitionProbability( line ), 1.0 );
    lwt2.addTransition( line2 );
    EXPECT_EQ( lwt2.getTransitionProbability( line ), 0.5 );
    EXPECT_EQ( lwt2.getTransitionProbability( line2 ), 0.5 );
}

TEST( test_binner, computes_transitions_among_single_thread ) {
    std::unique_ptr<std::vector<TokenWordPair>> line = std::make_unique<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "I";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "1019";
    line->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "41";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "428384";
    line->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12811";
    line->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line->push_back( twp );
    twp.tok = WORD;
    twp.word = "siteSelectorServer";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line->push_back( twp );
    twp.tok = WORD;
    twp.word = "cc";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "109";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = "]";
    line->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line->push_back( twp );
    twp.tok = WORD;
    twp.word = "adding";
    line->push_back( twp );
    twp.tok = WORD;
    twp.word = "connection";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "0";
    line->push_back( twp );

    std::unique_ptr<std::vector<TokenWordPair>> line_copy1 = std::make_unique<std::vector<TokenWordPair>>( *line );
    std::unique_ptr<std::vector<TokenWordPair>> line_copy2 = std::make_unique<std::vector<TokenWordPair>>( *line );

    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();
    buffer->addLine( std::move( line_copy1 ) );
    buffer->addLine( std::move( line_copy2 ) );
    binner.binEntriesInBuffer( std::move( buffer ) );

    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    BinKey bk;
    bk.num_words = 5;
    bk.num_params = 1;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 1 );

    LineWithTransitions &lwt = vec.at(0);
    std::shared_ptr<std::vector<TokenWordPair>> line_shr_ptr( std::move( line ) );
    EXPECT_EQ( *(lwt.getLine()), *line_shr_ptr );
    EXPECT_EQ( lwt.getTransitionProbability( line_shr_ptr ), 1.0 );
}

TEST( test_binner, multiple_transitions_and_lines_in_same_thread ) {
    std::unique_ptr<std::vector<TokenWordPair>> line = std::make_unique<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "I";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "1019";
    line->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "41";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "428384";
    line->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "12811";
    line->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line->push_back( twp );
    twp.tok = WORD;
    twp.word = "siteSelectorServer";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line->push_back( twp );
    twp.tok = WORD;
    twp.word = "cc";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "109";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = "]";
    line->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line->push_back( twp );
    twp.tok = WORD;
    twp.word = "adding";
    line->push_back( twp );
    twp.tok = WORD;
    twp.word = "connection";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "0";
    line->push_back( twp );
    std::unique_ptr<std::vector<TokenWordPair>> line_copy1 = std::make_unique<std::vector<TokenWordPair>>( *line );
    std::unique_ptr<std::vector<TokenWordPair>> line_copy2 = std::make_unique<std::vector<TokenWordPair>>( *line );

    std::unique_ptr<std::vector<TokenWordPair>> line2 = std::make_unique<std::vector<TokenWordPair>>();
    twp.tok = WORD;
    twp.word = "I";
    line2->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "1019";
    line2->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line2->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line2->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line2->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "41";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line2->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "428384";
    line2->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line2->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "12811";
    line2->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line2->push_back( twp );
    twp.tok = WORD;
    twp.word = "siteSelectorServer";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line2->push_back( twp );
    twp.tok = WORD;
    twp.word = "cc";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line2->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "109";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = "]";
    line2->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line2->push_back( twp );
    twp.tok = WORD;
    twp.word = "adding";
    line2->push_back( twp );
    twp.tok = WORD;
    twp.word = "NONMATCHING";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line2->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "0";
    line2->push_back( twp );

    std::unique_ptr<std::vector<TokenWordPair>> line2_copy1 = std::make_unique<std::vector<TokenWordPair>>( *line2 );
    std::unique_ptr<std::vector<TokenWordPair>> line2_copy2 = std::make_unique<std::vector<TokenWordPair>>( *line2 );

    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();
    buffer->addLine( std::move( line_copy1 ) );
    buffer->addLine( std::move( line_copy2 ) );
    buffer->addLine( std::move( line2_copy1 ) );
    buffer->addLine( std::move( line2_copy2 ) );

    binner.binEntriesInBuffer( std::move( buffer ) );

    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    BinKey bk;
    bk.num_words = 5;
    bk.num_params = 2;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );
    std::vector<LineWithTransitions> &vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 2 );

    LineWithTransitions &lwt = vec.at(0);
    std::shared_ptr<std::vector<TokenWordPair>> line_shr_ptr( std::move( line ) );
    std::shared_ptr<std::vector<TokenWordPair>> line2_shr_ptr( std::move( line2 ) );
    EXPECT_EQ( *(lwt.getLine()), *line_shr_ptr );
    EXPECT_EQ( lwt.getTransitionProbability( line_shr_ptr ), 0.5 );
    EXPECT_EQ( lwt.getTransitionProbability( line2_shr_ptr ), 0.5 );

    LineWithTransitions &lwt2 = vec.at(1);
    EXPECT_EQ( *(lwt2.getLine()), *line2_shr_ptr );
    EXPECT_EQ( lwt2.getTransitionProbability( line2_shr_ptr ), 1.0 );
}

TEST( test_binner, multiple_transitions_and_lines_multiple_threads_serdes ) {
    std::unique_ptr<std::vector<TokenWordPair>> line = std::make_unique<std::vector<TokenWordPair>>();
    TokenWordPair twp;
    twp.tok = WORD;
    twp.word = "I";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "1019";
    line->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "41";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "428384";
    line->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "12811";
    line->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line->push_back( twp );
    twp.tok = WORD;
    twp.word = "siteSelectorServer";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line->push_back( twp );
    twp.tok = WORD;
    twp.word = "cc";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "109";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = "]";
    line->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line->push_back( twp );
    twp.tok = WORD;
    twp.word = "adding";
    line->push_back( twp );
    twp.tok = WORD;
    twp.word = "connection";
    line->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "0";
    line->push_back( twp );
    std::unique_ptr<std::vector<TokenWordPair>> t12811_line_copy1 = std::make_unique<std::vector<TokenWordPair>>( *line );
    std::unique_ptr<std::vector<TokenWordPair>> t12811_line_copy2 = std::make_unique<std::vector<TokenWordPair>>( *line );

    std::unique_ptr<std::vector<TokenWordPair>> line2 = std::make_unique<std::vector<TokenWordPair>>();
    twp.tok = WORD;
    twp.word = "I";
    line2->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "1019";
    line2->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line2->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line2->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line2->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "41";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line2->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "428384";
    line2->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line2->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "12811";
    line2->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line2->push_back( twp );
    twp.tok = WORD;
    twp.word = "siteSelectorServer";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line2->push_back( twp );
    twp.tok = WORD;
    twp.word = "cc";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line2->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "109";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = "]";
    line2->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line2->push_back( twp );
    twp.tok = WORD;
    twp.word = "adding";
    line2->push_back( twp );
    twp.tok = WORD;
    twp.word = "NONMATCHING";
    line2->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line2->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "0";
    line2->push_back( twp );

    std::unique_ptr<std::vector<TokenWordPair>> t12811_line2_copy1 = std::make_unique<std::vector<TokenWordPair>>( *line2 );
    std::unique_ptr<std::vector<TokenWordPair>> t12811_line2_copy2 = std::make_unique<std::vector<TokenWordPair>>( *line2 );

    std::unique_ptr<std::vector<TokenWordPair>> line3 = std::make_unique<std::vector<TokenWordPair>>();
    twp.tok = WORD;
    twp.word = "I";
    line3->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "1019";
    line3->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line3->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line3->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line3->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line3->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line3->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "41";
    line3->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line3->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "428384";
    line3->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line3->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "12812";
    line3->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line3->push_back( twp );
    twp.tok = WORD;
    twp.word = "siteSelectorServer";
    line3->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line3->push_back( twp );
    twp.tok = WORD;
    twp.word = "cc";
    line3->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line3->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "109";
    line3->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = "]";
    line3->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line3->push_back( twp );
    twp.tok = WORD;
    twp.word = "adding";
    line3->push_back( twp );
    twp.tok = WORD;
    twp.word = "connection";
    line3->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line3->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "0";
    line3->push_back( twp );
    std::unique_ptr<std::vector<TokenWordPair>> t12812_line_copy1 = std::make_unique<std::vector<TokenWordPair>>( *line3 );
    std::unique_ptr<std::vector<TokenWordPair>> t12812_line_copy2 = std::make_unique<std::vector<TokenWordPair>>( *line3 );

    std::unique_ptr<std::vector<TokenWordPair>> line4 = std::make_unique<std::vector<TokenWordPair>>();
    twp.tok = WORD;
    twp.word = "I";
    line4->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "1019";
    line4->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line4->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line4->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line4->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "12";
    line4->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line4->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "41";
    line4->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line4->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "428384";
    line4->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line4->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "12812";
    line4->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line4->push_back( twp );
    twp.tok = WORD;
    twp.word = "siteSelectorServer";
    line4->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ".";
    line4->push_back( twp );
    twp.tok = WORD;
    twp.word = "cc";
    line4->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line4->push_back( twp );
    twp.tok = NUMBER;
    twp.word = "109";
    line4->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = "]";
    line4->push_back( twp );
    twp.tok = WHITE_SPACE;
    twp.word = " ";
    line4->push_back( twp );
    twp.tok = WORD;
    twp.word = "adding";
    line4->push_back( twp );
    twp.tok = WORD;
    twp.word = "NONMATCHING";
    line4->push_back( twp );
    twp.tok = PUNCTUATION;
    twp.word = ":";
    line4->push_back( twp );
    twp.tok = ABSTRACTED_VALUE;
    twp.word = "0";
    line4->push_back( twp );

    std::unique_ptr<std::vector<TokenWordPair>> t12812_line2_copy1 = std::make_unique<std::vector<TokenWordPair>>( *line4 );
    std::unique_ptr<std::vector<TokenWordPair>> t12812_line2_copy2 = std::make_unique<std::vector<TokenWordPair>>( *line4 );

    ParseBufferEngine pbe_in;
    Binner binner( &pbe_in );

    std::unique_ptr<ParseBuffer> buffer = std::make_unique<ParseBuffer>();
    buffer->addLine( std::move( t12811_line_copy1 ) );
    buffer->addLine( std::move( t12812_line2_copy1 ) );
    buffer->addLine( std::move( t12812_line2_copy2 ) );
    buffer->addLine( std::move( t12811_line_copy2 ) );
    buffer->addLine( std::move( t12812_line_copy1 ) );
    buffer->addLine( std::move( t12811_line2_copy1 ) );
    buffer->addLine( std::move( t12812_line_copy2 ) );
    buffer->addLine( std::move( t12811_line2_copy2 ) );

    //Line1:
    //Line -> Line
    //Line -> Line2
    //Line -> Line
    //So 2/3 -> Line, 1/3 -> Line2

    //Line2:
    //Line2 -> Line2
    //Line2 -> Line
    //Line2 -> Line2
    //So 2/3 -> Line2, 1/3 -> Line

    binner.binEntriesInBuffer( std::move( buffer ) );

    std::unordered_map<BinKey, Bin, BinKeyHasher> &map = binner.getUnderlyingMap();

    BinKey bk;
    bk.num_words = 5;
    bk.num_params = 2;
    auto search = map.find( bk );

    EXPECT_NE( search, map.end() );

    std::vector<LineWithTransitions> &vec = search->second.getBinVector();

    std::shared_ptr<std::vector<TokenWordPair>> line_shr_ptr( std::move( line ) );
    std::shared_ptr<std::vector<TokenWordPair>> line2_shr_ptr( std::move( line2 ) );
    std::shared_ptr<std::vector<TokenWordPair>> line3_shr_ptr( std::move( line3 ) );
    std::shared_ptr<std::vector<TokenWordPair>> line4_shr_ptr( std::move( line4 ) );

    EXPECT_EQ( vec.size(), 2 );
    LineWithTransitions &lwt = vec.at(0);
    EXPECT_EQ( *(lwt.getLine()), *line_shr_ptr );
    EXPECT_DOUBLE_EQ( lwt.getTransitionProbability( line_shr_ptr ), (double) 2 / 3 );
    EXPECT_DOUBLE_EQ( lwt.getTransitionProbability( line2_shr_ptr ), (double) 1 / 3 );

    LineWithTransitions &lwt2 = vec.at(1);
    EXPECT_EQ( *(lwt2.getLine()), *line4_shr_ptr );
    EXPECT_EQ( lwt2.getTransitionProbability( line_shr_ptr ), (double) 1 / 3 );
    EXPECT_EQ( lwt2.getTransitionProbability( line2_shr_ptr ), (double) 2 / 3 );

    //Serialize
    std::ofstream os;
    unlink( "test_serialize.txt" );
    os.open( "test_serialize.txt", std::ofstream::out );
    binner.serialize( os );
    os.close();

    //Deserialize
    std::ifstream is;
    is.open( "test_serialize.txt", std::ifstream::in );

    Binner binner2( &pbe_in );
    std::cout << "Going to deserialize" << std::endl;
    binner2.deserialize( is );
    std::cout << "Done deserializing..." << std::endl;

    map = binner2.getUnderlyingMap();
    search = map.find( bk );

    EXPECT_NE( search, map.end() );
    vec = search->second.getBinVector();
    EXPECT_EQ( vec.size(), 2 );

    LineWithTransitions &lwt3 = vec.at(0);
    EXPECT_EQ( *(lwt3.getLine()), *line_shr_ptr );
    EXPECT_DOUBLE_EQ( lwt3.getTransitionProbability( line_shr_ptr ), (double) 2 / 3 );
    EXPECT_DOUBLE_EQ( lwt3.getTransitionProbability( line2_shr_ptr ), (double) 1 / 3 );

    LineWithTransitions &lwt4 = vec.at(1);
    EXPECT_EQ( *(lwt4.getLine()), *line4_shr_ptr );
    EXPECT_EQ( lwt4.getTransitionProbability( line_shr_ptr ), (double) 1 / 3 );
    EXPECT_EQ( lwt4.getTransitionProbability( line2_shr_ptr ), (double) 2 / 3 );

}
