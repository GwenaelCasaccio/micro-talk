Object subclass: #Point [
    | x y |

    x: newX [ x := newX ]

    y: newY [ y := newY ]

    + aPoint [
        ^ Point new
            x: x + aPoint x;
            y: y + aPoint y
    ]
]
