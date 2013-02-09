package count.agent;

/**
 * This utility class can be used to indicate the instantiation of an object.
 *
 * @author Karl Bennett
 */
public final class NewEvent {

    private NewEvent() {
    }

    /**
     * A private native method that is called pass the new object into the attached {@code count-agent}.
     *
     * @param object the object that has just been instantiated.
     */
    private static native void nativeNewEvent(Object object);

    /**
     * Indicate that the supplied {@link Object} has been instantiated.
     *
     * @param object the object that has just been instantiated.
     */
    public static void newEvent(Object object) {

        nativeNewEvent(object);
    }
}
