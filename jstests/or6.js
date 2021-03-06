t = db.jstests_or6;
t.drop();

t.ensureIndex( {a:1} );

assert.eq.automsg( "2", "t.find( {$or:[{a:{$gt:2}},{a:{$gt:0}}]} ).explain().clauses[ 1 ].indexBounds[ 0 ][ 1 ].a" );
assert.eq.automsg( "2", "t.find( {$or:[{a:{$lt:2}},{a:{$lt:4}}]} ).explain().clauses[ 1 ].indexBounds[ 0 ][ 0 ].a" );

assert.eq.automsg( "2", "t.find( {$or:[{a:{$gt:2,$lt:10}},{a:{$gt:0,$lt:5}}]} ).explain().clauses[ 1 ].indexBounds[ 0 ][ 1 ].a" );
assert.eq.automsg( "0", "t.find( {$or:[{a:{$gt:2,$lt:10}},{a:{$gt:0,$lt:15}}]} ).explain().clauses[ 1 ].indexBounds[ 0 ][ 0 ].a" );
assert.eq.automsg( "15", "t.find( {$or:[{a:{$gt:2,$lt:10}},{a:{$gt:0,$lt:15}}]} ).explain().clauses[ 1 ].indexBounds[ 0 ][ 1 ].a" );

// no separate clauses
assert.eq.automsg( "null", "t.find( {$or:[{a:{$gt:2,$lt:10}},{a:{$gt:3,$lt:5}}]} ).explain().clauses" );

assert.eq.automsg( "20", "t.find( {$or:[{a:{$gt:2,$lt:10}},{a:{$gt:3,$lt:5}},{a:{$gt:20}}]} ).explain().clauses[ 1 ].indexBounds[ 0 ][ 0 ].a" );

assert.eq.automsg( "null", "t.find( {$or:[{a:1},{b:2}]} ).hint( {a:1} ).explain().clauses" );
assert.eq.automsg( "2", "t.find( {$or:[{a:1},{a:3}]} ).hint( {a:1} ).explain().clauses.length" );
assert.eq.automsg( "'BasicCursor'", "t.find( {$or:[{a:1},{a:3}]} ).hint( {$natural:1} ).explain().cursor" );

t.ensureIndex( {b:1} );
assert.eq.automsg( "2", "t.find( {$or:[{a:1,b:5},{a:3,b:5}]} ).hint( {a:1} ).explain().clauses.length" );

t.drop();

t.ensureIndex( {a:1,b:1} );
assert.eq.automsg( "2", "t.find( {$or:[{a:{$in:[1,2]},b:5}, {a:2,b:6}]} ).explain().clauses.length" );
assert.eq.automsg( "2", "t.find( {$or:[{a:{$gt:1,$lte:2},b:5}, {a:2,b:6}]} ).explain().clauses.length" );
assert.eq.automsg( "null", "t.find( {$or:[{a:{$gt:1,$lte:3},b:5}, {a:2,b:6}]} ).explain().clauses" );
assert.eq.automsg( "null", "t.find( {$or:[{a:{$in:[1,2]}}, {a:2}]} ).explain().clauses" );