package me.clarius.sdk.cast.example;

import java.util.Optional;
import java.util.function.Function;

public class Utils {
    /**
     * Convert a string to an integer.
     *
     * @param from      input string.
     * @param transform a conversion function, for example Long::parseLong.
     * @param <T>       the integer type, for example Long.
     * @return the converted integer or an empty optional if the conversion failed.
     */
    static <T> Optional<T> maybeInteger(final CharSequence from, Function<String, T> transform) {
        try {
            final String fromString = String.valueOf(from);
            final T ret = transform.apply(fromString);
            return Optional.of(ret);
        } catch (NumberFormatException e) {
            return Optional.empty();
        }
    }

    /**
     * Convert a string to a long.
     *
     * @param from input string.
     * @return the converted long or an empty optional if the conversion failed.
     */
    static Optional<Long> maybeLong(final CharSequence from) {
        return maybeInteger(from, Long::parseLong);
    }

    /**
     * Convert a string to an int.
     *
     * @param from input string.
     * @return the converted int or an empty optional if the conversion failed.
     */
    static Optional<Integer> maybeInt(final CharSequence from) {
        return maybeInteger(from, Integer::parseInt);
    }
}
